#include "readmanager.hpp"

#include <trek/common/applog.hpp>
#include <trek/common/stringbuilder.hpp>
#include <trek/common/timeprint.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <chrono>
#include <iomanip>

using std::endl;

using std::string;
using std::istream;
using std::exception;
using std::chrono::seconds;
using std::vector;
using std::setfill;
using std::setw;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::runtime_error;

using trek::Log;
using trek::StringBuilder;
using trek::data::NevodPackage;

using nlohmann::json;

namespace fs = boost::filesystem;

ReadManager::ReadManager(ModulePtr module,
                         const Settings& settings,
				         const ChannelConfig& config)
	: mModule(module),
	  mEncoder(formDir(settings), settings.eventsPerFile, config),
	  mNevodReceiver(settings.infoIp, settings.infoPort) {
    outputMeta(formDir(settings), settings, *module);
}

void ReadManager::run() {
	resetPackageCount();
	resetTriggerCount();
	if(mModule == nullptr || !mModule->isOpen())
		throw runtime_error("ReadManager::run tdc is not init");
	mNevodReceiver.onRecv([this](PackageReceiver::ByteVector& nevodBuffer) {
		try {
			handleNevodPackage(nevodBuffer, mNevodPackage);
			increasePackageCount();
			mModule->read(mBuffer);
			increaseTriggerCount(mBuffer.size());
			mEncoder.encode(mBuffer, mNevodPackage);
		} catch(const std::exception& e) {
			Log::instance() << "ReadManager: Failed handle buffer " << e.what() << std::endl;
		}
	});
    mNevodReceiver.start();
}

ReadManager::~ReadManager() {
	stop();
}

void ReadManager::stop() {
	mNevodReceiver.stop();
}

double ReadManager::getTriggerFrequency() const {
	return double(mTriggerCount) / duration_cast<seconds> (SystemClock::now() - mStartPoint).count();
}

double ReadManager::getPackageFrequency() const {
	return double(mPackageCount) / duration_cast<seconds> (SystemClock::now() - mStartPoint).count();
}

void ReadManager::increasePackageCount() {
	++mPackageCount;
}

void ReadManager::increaseTriggerCount(uintmax_t val) {
	mTriggerCount += val;
}

void ReadManager::resetPackageCount() {
	mPackageCount = 0;
}

void ReadManager::resetTriggerCount() {
	mTriggerCount = 0;
}

void ReadManager::handleNevodPackage(PackageReceiver::ByteVector& buffer, NevodPackage& nvdPkg) {
	struct membuf : public std::streambuf {
		membuf(char* p, size_t n) {
			setg(p, p, p + n);
			setp(p, p + n);
		}
	} tempBuffer(buffer.data(), buffer.size());
	istream stream(&tempBuffer);
	trek::deserialize(stream, nvdPkg);
	if(memcmp(nvdPkg.keyword, "TRACK ", sizeof(nvdPkg.keyword)) != 0)
		throw runtime_error("ReadManager::handleNevodPackage invalid package");
}

void ReadManager::outputMeta(const string& dirName, const Settings& settings, Tdc& module) {
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    stream.open(dirName + "/meta", stream.binary);
    stream << "Run: " << settings.nRun << '\n';
    stream << "Time: " << std::chrono::system_clock::now();
    stream << "TDC: " << module.name() << '\n';
    auto tdcSettings = module.settings();
    stream << "windowWidth:   " << tdcSettings.windowWidth << '\n';
    stream << "windowOffset:  " << tdcSettings.windowOffset << '\n';
    stream << "lsb:           " << tdcSettings.lsb << '\n';
    stream << "edgeDetection: " << int(tdcSettings.edgeDetection) << '\n';
}

string ReadManager::formDir(const Settings& settings) {
	string dir = StringBuilder() << settings.writeDir << "/run_" << setw(5) << setfill('0') << settings.nRun;
	if(!fs::exists(dir))
		fs::create_directories(dir);
	else if(!fs::is_directory(dir))
		throw runtime_error("ProcessController::fromWriteDir invalid dir");
	return dir;
}

void ReadManager::Settings::unMarshal(const json& doc) {
	nRun = doc.at("number_of_run");
	eventsPerFile = doc.at("events_per_file");
	writeDir = doc.at("write_dir").get<string>();
	infoIp = doc.at("info_pkg_ip").get<string>();
	infoPort = doc.at("info_pkg_port");
}

json ReadManager::Settings::marshal() const {
	return {
		{"number_of_run", nRun},
		{"events_per_file", eventsPerFile},
		{"write_dir", writeDir},
		{"info_pkg_ip", infoIp},
		{"info_pkg_port", infoPort},
	};
}
