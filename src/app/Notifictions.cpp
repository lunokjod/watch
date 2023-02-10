#include <Arduino.h>
#include <LilyGoWatch.h>
#include <Arduino_JSON.h>

#include "Notifications.hpp"
#include "UI/UI.hpp"
#include "LogView.hpp"

extern JSONVar localCloudNetworkHome;

NotificacionsApplication::~NotificacionsApplication() {
    if ( nullptr != msgfrom ) { free(msgfrom); }
    if ( nullptr != msgtitle ) { free(msgtitle); }
    if ( nullptr != msgbody ) {  free(msgbody); }
}

NotificacionsApplication::NotificacionsApplication(const char *subject,const char *from,const char *body) {
    msgfrom=(char*)ps_malloc(sizeof(from)+1);
    msgtitle=(char*)ps_malloc(sizeof(subject)+1);
    msgbody=(char*)ps_malloc(sizeof(body)+1);
    if ( nullptr != msgfrom ) {
        sprintf(msgfrom,"%s",from);
    }
    if ( nullptr != msgtitle ) {
        sprintf(msgtitle,"%s",subject);
    }
    if ( nullptr != msgbody ) {
        sprintf(msgbody,"%s",body);
    }
    //lAppLog("MESSAGE TO SHOW:\n FROM: '%s' Subject: '%s' Body: '%s'\n",msgfrom,msgtitle,msgbody);
    Tick();
}

bool NotificacionsApplication::Tick() {
    //UINextTimeout = millis()+UITimeout;
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        //lAppLog("MESSAGE TO SHOW:\n FROM: '%s' Subject: '%s' Body: '%s'\n",msgfrom,msgtitle,msgbody);
        canvas->setTextFont(1);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(text));
        canvas->setTextDatum(TL_DATUM);
        if ( nullptr != msgfrom ) { canvas->drawString(msgfrom, 35, 5); }
        canvas->setTextSize(3);
        if ( nullptr != msgtitle ) { canvas->drawString(msgtitle, 5, 35); }
        canvas->setTextSize(2);
        if ( nullptr != msgbody ) { canvas->drawString(msgbody, 15, 85); }

        TemplateApplication::Tick();
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}