#ifndef OTA_Handler_H
#define OTA_Handler_H

class OTAHandlerCallbacks;
class OTAHandler {
    public:
        OTAHandler();
        void setup(const char* ESP_ID, const char* ESP_PASS);
        void setCallbacks(OTAHandlerCallbacks* pCallbacks);
        void loop();
        void setBaud(int baud);
    private:
        OTAHandlerCallbacks* m_pOTAHandlerCallbacks = nullptr;
        const char* _ESP_ID;
        const char* _ESP_PASS;
        int _baud;
};

class OTAHandlerCallbacks {
public:
    virtual ~OTAHandlerCallbacks() {};
	virtual void onProgress(OTAHandler* pOtaHandler, unsigned int progress, unsigned int total);
};

#endif
