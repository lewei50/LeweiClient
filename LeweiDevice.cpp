#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>

#include <LeweiClient.h>

LeWeiDevice::LeWeiDevice(
        const char *id,
        const char *type,
        const char *name) :
    id(id),
    type(type),
    name(name),
    status("ok"),
    next(NULL)
{
}

bool LeWeiDevice::init(void)
{
    return true;
}

bool LeWeiSensor::getValue(int *val)
{
    return false;
}

bool LeWeiSensor::getValue(double *val)
{
    return false;
}

LeWeiAnalogSensor::LeWeiAnalogSensor(
        const char *id,
        const char *type,
        const char *name,
        int pin) : LeWeiSensor(id, type, name), _pin(pin)
{
}

bool LeWeiAnalogSensor::getValue(int *i)
{
    *i = analogRead(_pin);
    return true;
}

DHT11::DHT11(
        int pin) : _pin(pin), _timestamp(0)
{
}

bool DHT11::init(void)
{
    DDRC |= _BV(_pin);
    PORTC |= _BV(_pin);
    return true;
};

uint8_t DHT11::read_dht11_dat()
{
    uint8_t result=0;
    for(uint8_t i = 0; i < 8; i++){
        while(!(PINC & _BV(_pin)))
            ;  // wait for 50us

        delayMicroseconds(30);
        if(PINC & _BV(_pin))
            result |=(1<<(7-i));

        while((PINC & _BV(_pin)))
            ;  // wait '1' finish
    }
    return result;
}

bool DHT11::trans(void)
{
    if (millis() - _timestamp < 1000)
    {
        Serial.println("double read dht11");
        return true;
    }

    uint8_t dht11_in;
    bool res = false;
    uint8_t dht11_check_sum = 0;
    // start condition
    // 1. pull-down i/o pin from 18ms
    PORTC &= ~_BV(_pin);
    delay(18);
    PORTC |= _BV(_pin);
    delayMicroseconds(40);

    DDRC &= ~_BV(_pin);
    delayMicroseconds(40);

    dht11_in = PINC & _BV(_pin);

    if (dht11_in) {
        Serial.println("dht11 start condition 1 not met");
        goto _dht11_exit;
    }
    delayMicroseconds(80);

    dht11_in = PINC & _BV(_pin);

    if (!dht11_in) {
        Serial.println("dht11 start condition 2 not met");
        goto _dht11_exit;
    }
    delayMicroseconds(80);

    // now ready for data reception
    for (uint8_t i = 0; i < 5; i++)
    {
        data_buf[i] = read_dht11_dat();
        dht11_check_sum += data_buf[i];
    }

    DDRC |= _BV(_pin);
    PORTC |= _BV(_pin);

    // check check_sum
    if(data_buf[4] != dht11_check_sum - data_buf[4])
    {
        Serial.println("DHT11 checksum error");
        goto _dht11_exit;
    }

    _timestamp = millis();
    res = true;

_dht11_exit:
    // re-init
    init();
    return res;
}

LeWeiDHTHumSensor::LeWeiDHTHumSensor(
        const char *id,
        const char *type,
        const char *name,
        DHT11 *dht) : LeWeiSensor(id, type, name), _dht(dht)
{
};

bool LeWeiDHTHumSensor::getValue(double *i)
{
    if (!_dht->trans())
        return false;

    *i = _dht->data_buf[0] + (double)_dht->data_buf[1] / 256;
    return true;
}

LeWeiDHTTemprSensor::LeWeiDHTTemprSensor(
        const char *id,
        const char *type,
        const char *name,
        DHT11 *dht) : LeWeiSensor(id, type, name), _dht(dht)
{
}

bool LeWeiDHTTemprSensor::getValue(double *i)
{
    if (!_dht->trans())
        return false;

    *i = _dht->data_buf[2] + (double)_dht->data_buf[3] / 256;
    return true;
}

LeWeiPPDSensor::LeWeiPPDSensor(const char *id,
        const char *type,
        const char *name,
        int pin) : LeWeiSensor(id, type, name), _pin(pin)
{
}

bool LeWeiPPDSensor::init(void)
{
    pinMode(_pin, INPUT);

    return true;
}

bool LeWeiPPDSensor::getValue(double *i)
{
    const unsigned long sampletime_ms = 30000;
    unsigned long lowpulseoccupancy = 0;

    unsigned long stoptime = millis() + sampletime_ms;
    while (((signed long)millis() - (signed long)stoptime) < 0)
        lowpulseoccupancy += pulseIn(_pin, LOW);

    float ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    *i = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve

    return true;
}

LeWeiBH17xxSensor::LeWeiBH17xxSensor(
        const char *id,
        const char *type,
        const char *name,
        int addr) : LeWeiSensor(id, type, name), _addr(addr)
{
}

bool LeWeiBH17xxSensor::init(void)
{
    Wire.begin();
    Wire.beginTransmission(_addr);
    Wire.write(0x10); //1lx reolution 120ms
    Wire.endTransmission();
    delay(200);

    return true;
}

bool LeWeiBH17xxSensor::getValue(int *i)
{
    if(_BH17xx_Read() == 2)
    {
        *i=((buf[0] << 8) | buf[1])/1.2;
        delay(150);
        return true;
    }
    else
    {
        return false;
    }
}

int LeWeiBH17xxSensor::_BH17xx_Read(void)
{
    unsigned int i = 0;
    Wire.beginTransmission(_addr);
    Wire.requestFrom(_addr, 2);
    while(Wire.available() && (i < sizeof(buf)))
    {
        buf[i] = Wire.read();
        i++;
    }
    Wire.endTransmission();
    return i;
}

bool LeWeiActuator::updateValue(char* id,int val)
{
    return false;
}

bool LeWeiActuator::getValue(int *val)
{
    return false;
}

