#pragma once

#include <ESP8266WiFi.h>
#include <ESPAsyncWebSrv.h>
//#include <ESP8266mDNS.h>
#include <Task.h>

#include "robo.hpp"
#include "robo_state.hpp"
#include "robo_configuration.hpp"
#include "robo_log.hpp"

namespace robo {

static const char *wifi_name = "iPhone"; //"robotzone";
static const char *wifi_pass = "kekkeroni"; //robotzone";
//static const char *mdns_name = "martoaga";

class WebTask : public Task {
public:
  WebTask() : Task(),
    server(80),
    sse("/sse") {}
  
  void broadcastSSE(String message) {
    if(online) {
      sse.send(message.c_str(), NULL, millis(), 200);
    }
  }

private:
  bool online = false;
  AsyncWebServer server;
  AsyncEventSource sse;

protected:
  void setup() {
    LOG("web", "Hello!");

    // 1. Connect to wifi
    WiFi.begin(wifi_name, wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
      LOG("web", "Waiting for wifi...");
      delay(500);
    }

    LOG("web", "Connected! IP address: %s", WiFi.localIP().toString().c_str());

    // 2. Prepare the web server
    server.on("/", [&](AsyncWebServerRequest *req){
      req->send(200, "text/plain", "Hello, world!");
    });

    server.on("/state", [&](AsyncWebServerRequest *req) {
      AsyncResponseStream *res = req->beginResponseStream("text/plain");
      int n = req->params();
      for(int i = 0; i < n; i++) {
        state_set(req->getParam(i)->name(), req->getParam(i)->value());
      }
      for(int i = 0; i < num_state_items; i++) { // There has to be a better more encapsulated way to do this but i'm in a hurry
        String buf;
        if(state_items[i].type == state_item::INT) {
          buf = "state " + state_items[i].name + " " + *((int*)state_items[i].addr);
        } else if(state_items[i].type == state_item::FLOAT) {
          buf = "state " + state_items[i].name + " " + *((float*)state_items[i].addr);
        }
        res->println(buf);
      }
      req->send(res);
    });

    server.on("/config", [&](AsyncWebServerRequest *req) {
      AsyncResponseStream *res = req->beginResponseStream("text/plain");
      int n = req->params();
      for(int i = 0; i < n; i++) {
        config_set(req->getParam(i)->name(), req->getParam(i)->value());
      }
      if(n != 0)
        config_flush();
      for(int i = 0; i < num_config_items; i++) { // There has to be a better more encapsulated way to do this but i'm in a hurry
        String buf;
        if(config_items[i].type == state_item::INT) {
          buf = "config " + config_items[i].name + " " + *((int*)config_items[i].addr);
        } else if(config_items[i].type == state_item::FLOAT) {
          buf = "config " + config_items[i].name + " " + *((float*)config_items[i].addr);
        }
        res->println(buf);
      }
      req->send(res);
    });

    // Server-Sent events :)
    sse.onConnect([](AsyncEventSourceClient *client){
      client->send("hello!",NULL,millis(),1000);
    });
    server.addHandler(&sse);

    server.begin();
    online = true;
  }

  void loop() {
    // Broadcast state at 10Hz
    for(int i = 0; i < num_state_items; i++) { // There has to be a better more encapsulated way to do this but i'm in a hurry
      String buf;
      if(state_items[i].type == state_item::INT) {
        buf = "state " + state_items[i].name + " " + *((int*)state_items[i].addr);
      } else if(state_items[i].type == state_item::FLOAT) {
        buf = "state " + state_items[i].name + " " + *((float*)state_items[i].addr);
      }
      broadcastSSE(buf);
    }
    delay(100);
  }
};

WebTask webTask;

}
