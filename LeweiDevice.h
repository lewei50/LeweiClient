#ifndef __LEWEIDEVICE_H__
#define __LEWEIDEVICE_H__

/* This is a "abstract" class of all kind of devices.
 *
 * The only purpose of it is to gather general information. You should inherit
 * LeWeiSensor or LeWeiActuator in most of the cases.
 */
class LeWeiDevice
{
    public:
        virtual bool init(void);

    protected:
        LeWeiDevice(
                const char *id,
                const char *type,
                const char *name);

    private:
        const char *id;
        const char *type;
        const char *name;
        char       *status;

        LeWeiDevice *next;

        friend class LeWeiClient;
};

class LeWeiSensor : public LeWeiDevice
{
    public:
        LeWeiSensor(
                const char *id,
                const char *type,
                const char *name) : LeWeiDevice(id, type, name)
    {};

        virtual bool getValue(int *val);
        virtual bool getValue(double *val);
    private:
        LeWeiSensor();
};

class LeWeiAnalogSensor: public LeWeiSensor
{
    public:
        LeWeiAnalogSensor(
                const char *id,
                const char *type,
                const char *name,
                int pin);
        bool getValue(int*);
    private:
        int _pin;
};

class DHT11
{
    public:
        DHT11(int pin);
        bool init(void);
        bool trans(void);

        uint8_t data_buf[5];
    private:
        int _pin;
        unsigned long _timestamp;
        uint8_t read_dht11_dat();
};

class LeWeiDHTHumSensor: public LeWeiSensor
{
    public:
        LeWeiDHTHumSensor(
                const char *id,
                const char *type,
                const char *name,
                DHT11 *dht);
        bool getValue(double*);
    private:
        DHT11 *_dht;
};

class LeWeiDHTTemprSensor: public LeWeiSensor
{
    public:
        LeWeiDHTTemprSensor(
                const char *id,
                const char *type,
                const char *name,
                DHT11 *dht);
        bool getValue(double*);
    private:
        DHT11 *_dht;
};

class LeWeiPPDSensor: public LeWeiSensor
{
    public:
        LeWeiPPDSensor(
                const char *id,
                const char *type,
                const char *name,
                int pin);
        bool init(void);
        bool getValue(double*);
    private:
        int _pin;
};

class LeWeiBH17xxSensor: public LeWeiSensor
{
    public:
        LeWeiBH17xxSensor(
                const char *id,
                const char *type,
                const char *name,
                int addr);
        bool init(void);
        bool getValue(int*);

    private:
        int _addr;
        unsigned char buf[2];

        int _BH17xx_Read(void);
};

class LeWeiActuator : public LeWeiDevice
{
    public:
        LeWeiActuator(
                const char *id,
                const char *type,
                const char *name) : LeWeiDevice(id, type, name)
    {};

        virtual bool updateValue(char* id,int val);
        /** get current status(value) of the actuator.
         *
         * Don't confuse this with LeWeiSensor::getValue
         */
        virtual bool getValue(int *val);
};

#endif /* end of include guard: __LEWEIDEVICE_H__ */
