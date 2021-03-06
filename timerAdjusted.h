/*
    Многофункциональный программный таймер на системном таймере millis() для Arduino
    Документация: 
    GitHub: https://github.com/GyverLibs/TimerMs
    Возможности:
    - Режим таймера и периодичного выполнения
    - Подключение функции-обработчика
    - Сброс/запуск/перезапуск/остановка/пауза/продолжение отсчёта
    - Возможность форсировать переполнение таймера
    - Возврат оставшегося времени в мс, а также условных единицах 8 и 16 бит
    - Несколько функций получения текущего статуса таймера
    - Алгоритм держит стабильный период и не боится переполнения millis()
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0 - релиз
*/

#ifndef TimerMs_h
#define TimerMs_h
#include <Arduino.h>

class TimerMs
{
public:
    // (период, мс), (0 не запущен / 1 запущен), (режим: 0 период / 1 таймер)
    TimerMs(uint32_t prd = 1000, bool state = 0, bool mode = 0)
    {
        setTime(prd);
        if (state)
            start();
        _mode = mode;
    }
    void setTimerMode()
    { // установить в режим таймера (остановится после срабатывания)
        _mode = 1;
    }
    void setPeriodMode()
    { // установить в режим периода (перезапустится после срабатывания)
        _mode = 0;
    }
    void setTime(uint32_t prd)
    { // установить время
        _prd = (prd == 0) ? 1 : prd;
    }
    void attach(void (*handler)())
    { // подключить коллбэк
        _handler = *handler;
    }
    void detach()
    {
        _handler = NULL;
    }

    void start()
    { // запустить/перезапустить таймер
        _state = true;
        _tmr = uptime();
    }
    void restart()
    {
        start();
    }
    void resume()
    { // продолжить после остановки
        _state = true;
        _tmr = uptime() - _buf;
    }
    void stop()
    { // остановить/приостановить таймер
        _state = false;
        _buf = uptime() - _tmr;
    }
    void force()
    { // принудительно переполнить таймер
        _tmr = uptime() - _prd;
    }

    // в режиме периода однократно вернёт true при каждом периоде
    // в режиме таймера будет возвращать true при срабатывании
    bool tick()
    {
        if (_state)
            _buf = uptime() - _tmr;
        if (_state && _buf >= _prd)
        {
            if (!_mode)
                _tmr += _prd * (_buf / _prd);
            else
                stop();
            if (*_handler)
                _handler();
            _ready = 1;
            return true;
        }
        return false;
    }

    bool ready()
    { // однократно вернёт true при срабатывании (флаг)
        if (_ready)
        {
            _ready = 0;
            return true;
        }
        return false;
    }
    bool elapsed()
    { // всегда возвращает true при срабатывании
        return (uptime() - _tmr >= _prd);
    }
    bool active()
    { // работает ли таймер (start/resume)
        return _state;
    }
    bool status()
    { // elapsed+active: работает ли таймер + не сработал ли он
        return _state && !elapsed();
    }

    // остаток времени
    uint32_t timeLeft()
    { // остаток времени в мс
        return max(long(_prd - _buf), 0L);
    }
    uint8_t timeLeft8()
    { // остаток времени в 0-255
        return max(255 - _buf * 255l / _prd, 0ul);
    }
    uint16_t timeLeft16()
    { // остаток времени в 0-65535
        return max(65535 - _buf * 65535l / _prd, 0ul);
    }

    uint32_t uptime()
    { // на случай использования в других фреймворках
        return millis();
    }

private:
    uint32_t _tmr = 0, _prd = 1000, _buf = 0;
    bool _state = 0, _mode = 0, _ready = 0;
    void (*_handler)() = NULL;
};
#endif
