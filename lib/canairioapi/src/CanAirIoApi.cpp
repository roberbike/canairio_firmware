#include <HTTPClient.h>
#include "CanAirIoApi.hpp"

CanAirIoApi::CanAirIoApi(bool debug)
{
    dev=debug;
}

CanAirIoApi::~CanAirIoApi()
{
}

void CanAirIoApi::configure(const char nameId[], const char sensorId[], const char endpoint[],const char host[],const uint16_t port)
{   
    id = new char[strlen(nameId)+1];
    strcpy(id,nameId); 
    devId = new char[strlen(sensorId)+1];
    strcpy(devId,sensorId); 
    url = new char[strlen(endpoint)+1];
    strcpy(url,endpoint);
    ip = new char[strlen(host)+1];
    strcpy(ip,host);
    _port = port;
    if(dev)Serial.println("-->[API] configure with id: "+String(devId));
}

void CanAirIoApi::authorize(const char username[], const char password[])
{
    _username = new char[strlen(username)+1];
    strcpy(_username,username);
    _password = new char[strlen(password)+1];
    strcpy(_password,password);
    _isAuthorised = true;
    if(dev)Serial.println("-->[API] user:"+String(_username)+" pass:"+String(_password));
}

bool CanAirIoApi::write(uint16_t pm1, uint16_t pm25, uint16_t pm10, uint32_t gas, float aqs, float iaq, float hum, float tmp, float prs, float lat, float lon, float alt, float spd, int stime)
{   
    HTTPClient http;
    char uri[32];
    sprintf(uri, "/%s", url);

    if(dev)Serial.println("\n-->[API] target: "+String(ip)+":"+String(_port)+String(uri));

    if(_isSecure) {
       http.begin(ip, _port, uri, _cert);
    }
    else {
        http.begin(ip, _port, uri);
    }

    http.addHeader("Content-Type","application/json");
    http.addHeader("cache-control","no-cache");
        
    if(_isAuthorised) {
        http.setAuthorization(_username,_password);
    }

    const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(16);
    StaticJsonDocument <capacity> doc;

    JsonObject data = doc.createNestedObject();

    data["measurement"] = id;
    data["sensorId"] = devId;
    JsonObject fields = data.createNestedObject("fields");
    if(pm1>0)  fields["pm1"] = pm1;
    if(pm25>0) fields["pm25"] = pm25;
    if(pm10>0) fields["pm10"] = pm10;
    if(gas>0)  fields["gas"] = gas;
    if(aqs>0)  fields["aqs"] = aqs;
    if(iaq>0)  fields["iaq"] = aqs;
    if(hum!=0) fields["hum"] = hum;
    if(tmp!=0) fields["tmp"] = tmp;
    if(prs>0)  fields["prs"] = prs;
    if(lat!=0) fields["lat"] = lat;
    if(lon!=0) fields["lon"] = lon;
    if(alt!=0) fields["alt"] = alt;
    if(spd!=0) fields["spd"] = spd;
    if(stime>0)fields["stime"] = stime;

    if(dev){
        Serial.print("-->[API] payload:");
        serializeJson(doc, Serial);
    }

    String writeBuf;
    serializeJson(doc,writeBuf);
    _latestResponse = http.POST(writeBuf.c_str());
    if(dev)Serial.println("\n-->[API] response: "+String(_latestResponse));
    http.end();
    return _latestResponse == 200;
}

int CanAirIoApi::getResponse()
{
    return _latestResponse;
}

bool CanAirIoApi::isSecure()
{
    return _isSecure;
}
