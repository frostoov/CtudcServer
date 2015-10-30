#include "tdc.hpp"
#include "pciqbus.hpp"

class EmissEM1 : public Tdc {
	enum class Reg: uint16_t;
	class PchiStat;
	class Decoder;
public:
	EmissEM1(long address);
	void read(std::vector<EventHits>& buffer) override;
	const std::string& name() const override;
	Settings settings() override;
	bool isOpen() const override;
	void clear() override;

	void open(const std::string& devName);
	void close();

	uint8_t ctrl();
	uint16_t stat();
protected:
	void setPchi();
private:
	PciQbus mDev;
	const long mAddress;
};
