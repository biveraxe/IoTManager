//=======================================================================================================
// Это файл сенсора, в нем осуществляется чтение сенсора.
// для добавления сенсора вам нужно скопировать этот файл и заменить в нем текст ExamleModule на название вашего сенсора
// Название должно быть уникальным, коротким и отражать суть сенсора.

// Обязательные библиотеки из ядра IoTM
#include "Global.h"
#include "classes/IoTItem.h"

//!Здесь подключаем стороннюю библиотеку при необходимости (ExternalLibrary заменить)
//#include <ExternalLibrary.h> 
#include "ExternalLibrary.h" //удалить, только для примера. Внешние библиотеки правильно в <>  

//! Объяевляем класс IoTGpio для работы с GPIO 
extern IoTGpio IoTgpio;

//=========================================================================================================
//=========================================================================================================
//              Объявление сторонней библиотекит с использованием глобавльных объектов 
//=======================================================================================================
//! Объявляем стороннюю библиотеку при необходимости (ExternalLibrary заменить)
// !!! ЗДЕСЬ И ДАЛЕЕ libXX НАЗВАТЬ уникальным именем)
ExternalLibrary *libXX = nullptr;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
// instanceLibXX НАЗВАТЬ по наименованию модуля (instanceДатчикХ)
// ПРИ НЕОБХДИМОСТИ передаем любые нужные параметры для инициализации библиотеки (в данном случае PIN)
// !!! ВЫзвать данную функцию нужно хотябы один раз, 
//  но в каждом конструкторе класса модуля ExampleModule_A, ExampleModule_B и т.д.
//  или можно вывывать постоянно при обращении к библиотеке, типа: instanceLibXX().READ_LIB_DATA_OTHER();
ExternalLibrary *instanceLibXX(int pin)
{
    if (!libXX)
    { // Если библиотека ранее инициализировалась, то просто вернем указатель
        // Инициализируем библиотеку
        libXX = new ExternalLibrary();
        libXX->begin(pin); // При необходимости делаем begin библиотеке
    }
    return libXX;
}


//=========================================================================================================
//=========================================================================================================
//              Первый класс модуля для определения 1-го элемента (параметра)
//  Служит для запроса и отображения парметра/элемента датчика
// IoTManager система модульная: один парметр - один элемент (класс)
//
//=========================================================================================================
//=========================================================================================================
// ребенок       -       родитель
class ExampleModule_A : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    // Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    // впоследствии использовать их в loop и setup
    unsigned int _pin;
    // пользовательская переменная, в данном случае для считывания аналогового сигнала
    int adc;

public:
    //=======================================================================================================
    // setup()
    // это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    // Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    // Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    ExampleModule_A(String parameters) : IoTItem(parameters)
    {
        //Читаем пользовательскую переменную PIN, должна быть объявлена в в modeinfo.json
        _pin = jsonReadInt(parameters, "pin");
        // другой вариант чтения парметров модуля
        jsonRead(parameters, F("int"), _interval, false);
    }

    //=======================================================================================================
    // doByInterval() - основная функция периодической работы
    // это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    // а затем выполнить regEvent - это регистрация произошедшего события чтения
    // здесь так же доступны все переменные из секции переменных, и полученные в setup
    // если у сенсора несколько величин то делайте несколько regEvent
    // не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    // и выполнить за несколько тактов
    void doByInterval()
    {
        // Пример получения данных из библиотеки, где READ_LIB_DATA_OTHER - функция Вашей библиотеки
        value.valD = instanceLibXX(_pin)->READ_LIB_DATA_OTHER();
        // Здесь Наменование произвольным но понятным к какому модулю относится
        regEvent(value.valD, "ExampleModule"); // обязательный вызов хотяб один для регистрации события в ядре IoTM
    }

    //=======================================================================================================
    // loop(), если не нужно переопределять, удалить.
    // полный аналог loop() из arduino. Нужно помнить, что все модули имеют равный поочередный доступ к центральному loop(), поэтому, необходимо следить
    // за задержками в алгоритме и не создавать пауз. Кроме того, данная версия перегружает родительскую, поэтому doByInterval() отключается, если
    // не повторить механизм расчета интервалов.
    void loop()
    {
        // Пример получения данных с аналоговым датчиком
        adc = IoTgpio.analogRead(_pin);

        // Блок вызова doByInterval, так как если определили loop, то сам он не вызовится
        IoTItem::loop();
    }

    ~ExampleModule_A(){};
};

//=========================================================================================================
//=========================================================================================================
//              Второй класс модуля для определения 2-го элемента (параметра)
//  Делается по аналогии с первым классом, служит для запроса и отображения другого парметра если их несколько с одного датчика
// IoTManager система модульная: один парметр - один элемент (класс)
//
//  Содержит описание дополнительных методов onModuleOrder и execute
//=========================================================================================================
//=========================================================================================================
class ExampleModule_B : public IoTItem
{
private:
//Пользовательские переменные
    unsigned int _pin;
public:
    ExampleModule_B(String parameters) : IoTItem(parameters)
    {
        //Читаем пользовательскую переменную PIN, должна быть объявлена в в modeinfo.json
        _pin = jsonReadInt(parameters, "pin");
        //Можно инициализировать библиотеку один раз, а потом используем указатель
        instanceLibXX(_pin);
        libXX->READ_LIB_DATA_OTHER();
    }

    void doByInterval()
    {
        // Пример получения данных из библиотеки, где READ_LIB_DATA_OTHER - функция Вашей библиотеки
        value.valD = libXX->READ_LIB_DATA_OTHER();
        // Здесь Наменование произвольным но понятным к какому модулю относится
        regEvent(value.valD, "ExampleModule"); // обязательный вызов хотяб один для регистрации события в ядре IoTM
    }

//================ обработка кнопок из конфигурации ===================
    // Хук (переопределение виртуальной функции) для обработки кнопки (в value будут данные с собственной панели ввода)
    // Что бы кнопка была без поля ввода, нужно в modeinfo.json указать "btn-Example": nil
    void onModuleOrder(String &key, String &value)
    {
        if (key == "Example") // название кнопки btn-Example
        {
            SerialPrint("i", F("Sensor ExampleModule"), "User run calibration " + value);
            // ЧТО ТО Делаем
        }
    }

//================ обработка команд из сценария===================
    // Хук (переопределение виртуальной функции) для обработки команды из сценария (в param будут даныые переданные в функции в сценарии)
    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "expampleFunc")
        {
            if (param.size())
            {
                // Забираем данные из другого модуля по его ID
                // Если в сценарии  передадим id модуля
                String value = getItemValue(param[0].valS);
                // Что то делаем с этим параметром
                return {};
            }
        }
        else if (command == "expample2")
        {
            if (param.size() == 2)
            {
                SerialPrint("i", F("Sensor ExampleModule"), "expample2: " + param[0].valS + ", " + param[1].valS);
                // Передаем полученные данные на дальнейшую обработку
                // foo(param[0].valS, param[1].valS);
                return {};
            }
        }
        else if (command == "expampleAny")
        {
            if (param.size() >= 1)
            {
                int sizeOfParam = param.size();
                for (unsigned int i = 0; i < sizeOfParam; i++)
                {
                    SerialPrint("i", F("Sensor ExampleModule"), "expampleAny: " + param[i].valS);
                    // Что то делаем с каждым принятым значением
                    // foo(param[i].valD);
                }
            }
        }
        return {};
    }

    void foo(String logid, String value)
    {
        // Прсото пример кокой-то функции
    }

    ~ExampleModule_B(){};
};


//=========================================================================================================
//=========================================================================================================
//      Функция для связи модуля с ядром IoTM
// !!! ЗДЕСЬ getAPI_ИМЯ ИМЯ должно совпадать с "moduleName" из modeinfo.json
//  после замены названия сенсора, на функцию можно не обращать внимания
//  если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//  создание и контроль соответствующих глобальных переменных
//=========================================================================================================
//=========================================================================================================
void *getAPI_ExampleModule(String subtype, String param)
{
    if (subtype == F("ExampleModule_A"))
    { // !!! ЗДЕСЬ subtype ДОЛЖЕН СОВПАДАТЬ С subtype из modeinfo.json
        return new ExampleModule_A(param);
    }
    else if (subtype == F("ExampleModule_B"))
    { // !!! ЗДЕСЬ subtype ДОЛЖЕН СОВПАДАТЬ С subtype из modeinfo.json
        return new ExampleModule_B(param);
    }
    else
    {
        return nullptr;
    }
}
