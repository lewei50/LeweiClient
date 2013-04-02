#ifndef __LEWEICLIENT_H__
#define __LEWEICLIENT_H__

#include <Ethernet.h>

#include <LeweiDevice.h>

struct sdata;

class LeWeiClient
{
    public:
        enum flag {
            // these flags should be bit fields
            none = 0x00,
            isControlled = 0x01,
            internetAvailable = 0x02,
        };

        LeWeiClient(const char *user_key,
                    const char *gateway,
                    const char *name,
                    const char *description,
                    const char *apiAddress,
                    flag _flag);

        void registerSensor(LeWeiSensor &dev);
        void registerActuator(LeWeiActuator &dev);
        /** number of sensors registered */
        unsigned int nrSensors(void);
        /** number of actuators registered */
        unsigned int nrActuators(void);

        int uploadInfo(void);

        bool initDevices(void);

	void append(char *name, int val);
	void append(char *name, double val);
        void scanSensors(void);

        int sendLog(char *log);

        int beginServe(uint16_t port);

        int serve(void);

    private:
        const char *_user_key;
        const char *_gateway;
        const char *_name;
        const char *_description;
        const char *_apiAddress;
        flag _flag;

        EthernetClient _client;
        EthernetServer *_server;
        LeWeiSensor *_sensors;
        LeWeiActuator *_actuators;

	struct sdata *_sdata;
	unsigned int  _sdata_len;

        int send(struct sdata*, unsigned int size);
        size_t send_chunked(const void *data, size_t size);
        size_t send_chunked(const char*);
        void dump_actuator_status_chunked(LeWeiActuator *dev);

        int on_getAllSensors(void);
        int on_updateSensor(char *, char *);
};

#endif /* end of include guard: __LEWEICLIENT_H__ */
