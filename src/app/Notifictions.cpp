#include <Arduino.h>
#include <LilyGoWatch.h>
#include <Arduino_JSON.h>

#include "Notifications.hpp"
#include "UI/UI.hpp"

extern JSONVar localCloudNetworkHome;

NotificacionsApplication::~NotificacionsApplication() {
    
}
NotificacionsApplication::NotificacionsApplication() {

    // get notifications from lunoKloud
    if ( localCloudNetworkHome.hasOwnProperty("notifications") ) {
        JSONVar notificationsJSON = localCloudNetworkHome["notifications"];

        Serial.println("");
        Serial.println("");
        localCloudNetworkHome.printTo(Serial);
        Serial.println("");
        Serial.println("");


        int remaining = notificationsJSON.length();
        for (int i = 0; i < remaining; i++) {
            JSONVar currentNotification = notificationsJSON[i];
            Serial.printf("NOTIFICATION:  %d\n",i);
            Serial.printf("     Version:  %d\n",int(currentNotification["v"]));
            Serial.printf("        UUID: '%s'\n",(const char *)(currentNotification["uuid"]));
            Serial.printf("       title: '%s'\n",(const char *)currentNotification["title"]);
            Serial.printf("        body: '%s'\n",(const char *)currentNotification["body"]);
            Serial.printf("        date:  %d\n",int(currentNotification["date"]));
            Serial.printf("        from: '%s'\n",(const char *)currentNotification["from"]);
            Serial.printf("        read:  %s\n",((const char *)(currentNotification["read"])?"true":"false"));
        }
    }
}

bool NotificacionsApplication::Tick() {
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        nextRedraw=millis()+(1000/2);
        return true;
    }
    return false;
}