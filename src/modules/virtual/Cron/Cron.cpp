#include "NTP.h"
#include "Global.h"
#include "classes/IoTItem.h"

extern "C" {
#include "ccronexpr/ccronexpr.h"
}


class Cron : public IoTItem {
   private:
    bool _pause = false;
    String _format = "";
    cron_expr _expr;
    time_t _nextAlarm = 0;
    bool _isInit = false;
    int _tzOffset = 0;  // часовой пояс, с которым посчитан _nextAlarm
    unsigned long _lastUnixTime = 0;  // unixTime на прошлом тике, для обнаружения отката часов

    int getTzOffset() { return jsonReadInt(settingsFlashJson, F("timezone")); }

   public:
    Cron(String parameters): IoTItem(parameters) {
        jsonRead(parameters, F("formatNextAlarm"), _format);
        if (_time_isTrust) initCron();
    }

    void initCron() {
        const char* err = NULL;
        memset(&_expr, 0, sizeof(_expr));
        
        cron_parse_expr(value.valS.c_str(), &_expr, &err);
        if (err) {
            _pause = true;
            _nextAlarm = 0;
            memset(&_expr, 0, sizeof(_expr));
            SerialPrint("E", "Cron", F("The Cron string did not apply."), _id);
        } else 
            updateNextAlarm(true);
    }

    void updateNextAlarm(bool forced) {
        if (!_pause && _time_isTrust) {
            if (forced || (_nextAlarm <= gmtTimeToLocal(unixTime))) {
                // update alarm if next trigger is not yet in the future
                _nextAlarm = cron_next(&_expr, gmtTimeToLocal(unixTime));
                _tzOffset = getTzOffset();
                _isInit = true;
            }
        }
    }

    String getNextAlarmF() {
        if (_pause) return "Pause";
        if (!_time_isTrust) return "No time";
        struct tm* timeinfo;
        char buffer [80];  
        timeinfo = localtime(&_nextAlarm); 
        strftime(buffer, 80, _format.c_str(), timeinfo); 
        return buffer;
    }

    String getValue() {
        return getNextAlarmF();
    }

    void setValue(const IoTValue& Value, bool genEvent) {
        if (!_time_isTrust) return;

        value = Value;
        _pause = false;
        initCron();
        
        if (_needSave) {
            jsonWriteStr_(valuesFlashJson, _id, value.valS);
            needSaveValues = true;
        }

        bool _needSaveBak = _needSave;
        _needSave = false;
        regEvent(getNextAlarmF(), F("Cron alarm"), false, false);   
        _needSave = _needSaveBak;
    }

    void doByInterval() {
        if (!_time_isTrust) return;
        if (!_isInit) initCron();
        // часовой пояс сменился или часы откатились назад (NTP, ручная установка) -
        // пересчитываем следующую сработку без генерации события.
        // Скачок вперёд не обрабатываем: пропущенная сработка выполнится один раз вдогонку
        bool tzChanged = _tzOffset != getTzOffset();
        bool timeBack = _lastUnixTime != 0 && unixTime + 60 < _lastUnixTime;
        _lastUnixTime = unixTime;
        if (_isInit && !_pause && (tzChanged || timeBack)) {
            SerialPrint("i", "Cron", tzChanged ? F("Timezone changed, recalculating next alarm") : F("Time moved back, recalculating next alarm"), _id);
            updateNextAlarm(true);
            bool _needSaveBak = _needSave;
            _needSave = false;
            regEvent(getNextAlarmF(), F("Cron alarm"), false, false);
            _needSave = _needSaveBak;
        }
        if (!_pause && (gmtTimeToLocal(unixTime) >= _nextAlarm)) {
            updateNextAlarm(true);
            bool _needSaveBak = _needSave;
            _needSave = false;
            regEvent(getNextAlarmF(), F("Cron alarm"));   
            _needSave = _needSaveBak;
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "stop") { 
            _pause = true;
        } else if (command == "continue") {
            _pause = false;
            updateNextAlarm(false);
        }

        bool _needSaveBak = _needSave;
        _needSave = false;
        regEvent(getNextAlarmF(), F("Cron alarm"), false, false);   
        _needSave = _needSaveBak;

        return {}; 
    }

    ~Cron() {};
};

void* getAPI_Cron(String subtype, String param) {
    if (subtype == F("Cron")) {
        return new Cron(param);
    } else {
        return nullptr;
    }
}
