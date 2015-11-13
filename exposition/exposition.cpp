#include "exposition.hpp"

#include <trek/common/stringbuilder.hpp>
#include <trek/common/timeprint.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <chrono>
#include <iomanip>

using std::string;
using std::istream;
using std::exception;
using std::vector;
using std::setfill;
using std::setw;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::milliseconds;
using std::runtime_error;

using trek::StringBuilder;

using nlohmann::json;

namespace fs = boost::filesystem;

Exposition::Exposition(ModulePtr module,
                       const Settings& settings,
                       const ChannelConfig& config)
	: mModule(module),
	  mEncoder(formDir(settings), formPrefix(settings), settings.eventsPerFile, config) {
	outputMeta(formDir(settings), settings, *module);
}

void Exposition::run() {
	resetPackageCount();
	resetTriggerCount();
	mWorking = true;
	if(mModule == nullptr || !mModule->isOpen())
		throw runtime_error("Exposition::run tdc is not init");
	mStartPoint = system_clock::now();

	vector<Tdc::EventHits> buffer;
	while(mWorking) {
		mModule->read(buffer);
		if(!buffer.empty())
			mEncoder.write(buffer);
	}
}

Exposition::~Exposition() {
	stop();
}

void Exposition::stop() {
	mWorking = false;
}

uintmax_t Exposition::triggerCount() const {
	return mTriggerCount;
}

uintmax_t Exposition::packageCount() const {
	return mPackageCount;
}

uintmax_t Exposition::duration() const {
	return uintmax_t( duration_cast<milliseconds>(system_clock::now() - mStartPoint).count() );
}


void Exposition::increasePackageCount() {
	++mPackageCount;
}

void Exposition::increaseTriggerCount(uintmax_t val) {
	mTriggerCount += val;
}

void Exposition::resetPackageCount() {
	mPackageCount = 0;
}

void Exposition::resetTriggerCount() {
	mTriggerCount = 0;
}

void Exposition::outputMeta(const string& dirName, const Settings& settings, Tdc& module) {
	std::ofstream stream;
	stream.exceptions(stream.failbit | stream.badbit);
	stream.open(dirName + "/meta", stream.binary);
	stream << "Run: " << settings.nRun << '\n';
	stream << "Time: " << std::chrono::system_clock::now() << '\n';
	stream << "TDC: " << module.name() << '\n';
	auto tdcSettings = module.settings();
	stream << "windowWidth:   " << tdcSettings.windowWidth << '\n';
	stream << "windowOffset:  " << tdcSettings.windowOffset << '\n';
	stream << "lsb:           " << tdcSettings.lsb << '\n';
	stream << "edgeDetection: " << int(tdcSettings.edgeDetection) << '\n';
}

string Exposition::formDir(const Settings& settings) {
	string dir = StringBuilder() << settings.writeDir << "/run_" << setw(5) << setfill('0') << settings.nRun;
	if(!fs::exists(dir))
		fs::create_directories(dir);
	else if(!fs::is_directory(dir))
		throw runtime_error("ProcessController::fromWriteDir invalid dir");
	return dir;
}

string Exposition::formPrefix(const Settings& settings) {
	return StringBuilder() << "ctudc_" << setw(5) << setfill('0') << settings.nRun << '_';
}


void Exposition::Settings::unMarshal(const json& doc) {
	nRun = doc.at("number_of_run");
	eventsPerFile = doc.at("events_per_file");
	writeDir = doc.at("write_dir").get<string>();
	infoIp = doc.at("info_pkg_ip").get<string>();
	infoPort = doc.at("info_pkg_port");
}

json Exposition::Settings::marshal() const {
	return {
		{"number_of_run", nRun},
		{"events_per_file", eventsPerFile},
		{"write_dir", writeDir},
		{"info_pkg_ip", infoIp},
		{"info_pkg_port", infoPort},
	};
}
