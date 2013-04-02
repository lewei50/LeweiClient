#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include <LeweiClient.h>

#ifdef __ART__
#define malloc      rt_malloc
#define realloc     rt_realloc
#define free        rt_free

#define DEBUG_PRINTF     rt_kprintf
#define DEBUG_PRINTS(s)  rt_kprintf(s)
#define DEBUG_PRINTSS(s) rt_kprintf(s)
#else
// taken from http://playground.arduino.cc/Main/Printf
#include <stdarg.h>
static void _print(const char *fmt, ...)
{
    char tmp[64];
    va_list args;
    va_start(args, fmt );
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    Serial.print(tmp);
}
#define DEBUG_PRINTF     _print
#define DEBUG_PRINTS(s)   Serial.print(s)
#define DEBUG_PRINTSS(s)  Serial.print(F(s))
#endif

//#define DEBUG_PRINTF(...)

const char *server = "open.lewei50.com";
//IPAddress  server_ip(121, 197, 10, 140);
const char *user_agent = "User-Agent: RT-Thread ART";

LeWeiClient::LeWeiClient(
        const char *user_key,
        const char *gateway,
        const char *name,
        const char *description,
        const char *apiAddress,
        enum flag flag) :
    _user_key(user_key),
    _gateway(gateway),
    _name(name),
    _description(description),
    _apiAddress(apiAddress),
    _flag(flag),
    _server(NULL),
    _sensors(NULL),
    _actuators(NULL),
    _sdata(NULL),
    _sdata_len(0)
{
}

size_t LeWeiClient::send_chunked(const void *data, size_t size)
{
    size_t res;
    char buf[4];
    if (size > (1 << 4*(sizeof(buf)-1)))
    {
        DEBUG_PRINTSS("too large data chnuk, max ");
        DEBUG_PRINTS(1 << 4*(sizeof(buf)-1));
        DEBUG_PRINTSS(" allowed\n");
        return 0;
    }

    snprintf(buf, sizeof(buf), "%x", size);
    _client.println(buf);
    res = _client.write((uint8_t*)data, size);
    _client.println();

    return res;
}

size_t LeWeiClient::send_chunked(const char *str)
{
    return send_chunked(str, strlen(str));
}

int LeWeiClient::uploadInfo(void)
{
    if (!_client.connect(server, 80))
    {
        DEBUG_PRINTSS("connect failed in uploadInfo\n");
        _client.stop();
        return -1;
    }

#define P(d) _client.print(d);
#define PN(d) _client.println(d);
#define PS(d) _client.print(F(d));
#define PSN(d) _client.println(F(d));
#define S(d) send_chunked(d);
    // send head.
    PS("POST /api/V1/Gateway/Update/")P(_gateway)PS(" HTTP/1.1\r\n");
    PS("userkey: ")PN(_user_key);
    PS("Host: open.lewei50.com \r\n");
    PN(user_agent);
    PSN("Transfer-Encoding: chunked");
    PSN("Connection: close");
    PN();
    //{"name":"name",
    //"description":"desc",
    //"isControlled":"true",
    //"internetAvailable":"true",
    //"apiAddress":"http://192.168.1.103:6399/api"}
    S("{\"name\":\"")S(_name);
    S("\",\"description\":\"")S(_description);
    S("\",\"isControlled\":\"");
    if (_flag & LeWeiClient::isControlled)
    {
        S("true");
    }
    else
    {
        S("false");
    }
    S("\",\"internetAvailable\":\"");
    if (_flag & LeWeiClient::internetAvailable)
    {
        S("true");
    }
    else
    {
        S("false");
    }
    S("\",\"apiAddress\":\"")S(_apiAddress)S("\"}");
    S("");

#if 0
    delay(5000);
    uint8_t buf[16];
    int j = _client.read(buf, sizeof(buf)-1);
    int retry = 30;
    while (j < 0 && retry--)
    {
        DEBUG_PRINTSS("wait for resp\n");
        j = _client.read(buf, sizeof(buf)-1);
        delay(1000);
    }
    if (!retry)
    {
        DEBUG_PRINTSS("no data from server\n");
        return 1;
    }

    do {
        buf[j] = '\0';
        DEBUG_PRINTS((char*)buf);
    } while((j = _client.read(buf, sizeof(buf)-1)) > 0);
#endif
    _client.stop();

    return 0;
#undef P
#undef PN
#undef PS
#undef PSN
#undef S
}

void LeWeiClient::registerSensor(LeWeiSensor &dev)
{
    dev.next = _sensors;
    _sensors  = &dev;
}

unsigned int LeWeiClient::nrSensors(void)
{
    unsigned int nr = 0;
    for (LeWeiDevice *dev = _sensors;
         dev;
         dev = dev->next)
    {
        nr++;
    }
    return nr;
}

void LeWeiClient::registerActuator(LeWeiActuator &dev)
{
    dev.next  = _actuators;
    _actuators = &dev;
}

unsigned int LeWeiClient::nrActuators(void)
{
    unsigned int nr = 0;
    for (LeWeiDevice *dev = _actuators;
         dev;
         dev = dev->next)
    {
        nr++;
    }
    return nr;
}

bool LeWeiClient::initDevices(void)
{
    bool res = true;

    for (LeWeiDevice *dev = _sensors;
         dev;
         dev = dev->next)
    {
        res &= dev->init();
    }

    for (LeWeiDevice *dev = _actuators;
         dev;
         dev = dev->next)
    {
        res &= dev->init();
    }

    return res;
}

struct sdata {
    const char *id;
    char *val;
};

void LeWeiClient::append(char *name, int val)
{
	if (_sdata == NULL)
	{
		_sdata = (struct sdata*)malloc(sizeof(*_sdata));
		if (!_sdata)
		{
			DEBUG_PRINTSS("malloc return NULL\n");
			return;
		}
	}
	else
	{
		struct sdata *tmp = (struct sdata*)realloc(_sdata,
				                           sizeof(*_sdata)*(_sdata_len+1));
		if (!tmp)
		{
			DEBUG_PRINTSS("realloc return NULL\n");
			return;
		}
		_sdata = tmp;
	}
	_sdata[_sdata_len].id = name;
	_sdata[_sdata_len].val = (char*)malloc(8); // xxxx'\0'
	if (!_sdata[_sdata_len].val)
	{
		DEBUG_PRINTSS("malloc return NULL\n");
		return;
	}
	int val_len = snprintf(_sdata[_sdata_len].val, 8, "%d", val);
	if (val_len >= 8)
	{
		DEBUG_PRINTS(name);
		DEBUG_PRINTSS(" value too big: ");
		Serial.println(val);
	}

	_sdata_len++;
}

void LeWeiClient::append(char *name, double val)
{
	if (_sdata == NULL)
	{
		_sdata = (struct sdata*)malloc(sizeof(*_sdata));
		if (!_sdata)
		{
			DEBUG_PRINTSS("malloc return NULL\n");
			return;
		}
	}
	else
	{
		struct sdata *tmp = (struct sdata*)realloc(_sdata,
				                           sizeof(*_sdata)*(_sdata_len+1));
		if (!tmp)
		{
			DEBUG_PRINTSS("realloc return NULL\n");
			return;
		}
		_sdata = tmp;
	}
	_sdata[_sdata_len].id = name;
	_sdata[_sdata_len].val = (char*)malloc(10); // xx.xx'\0'
	if (!_sdata[_sdata_len].val)
	{
		DEBUG_PRINTSS("malloc return NULL\n");
		return;
	}
	int val_len = snprintf(_sdata[_sdata_len].val, 10,
			"%d.%02u", (int)val, (int)(abs(val)*100+0.5) % 100);
	if (val_len >= 10)
	{
		DEBUG_PRINTS(name);
		DEBUG_PRINTSS(" value too big: ");
		Serial.println(val);
		DEBUG_PRINTS(val_len);
	}

	_sdata_len++;
}

void LeWeiClient::scanSensors(void)
{
    unsigned int nrs = nrSensors();
    int intval;
    double douval;
    unsigned int i = _sdata_len;

    if (_sdata == NULL)
    {
	    Serial.println(F("first alloc _sdata in scanSensors"));
	    Serial.print(_sdata_len);
	    Serial.print(F(", "));
	    Serial.println(nrs);
	    _sdata = (struct sdata*)malloc(sizeof(*_sdata) * nrs);
	    if (!_sdata)
	    {
		    DEBUG_PRINTSS("malloc return NULL\n");
		    return;
	    }
    }
    else
    {
	    Serial.println(F("second alloc _sdata in scanSensors"));
	    struct sdata *tmp = (struct sdata*)realloc(_sdata,
			    sizeof(*_sdata)*(_sdata_len + nrs));
	    if (!tmp)
	    {
		    DEBUG_PRINTSS("realloc return NULL\n");
		    DEBUG_PRINTSS("restart the client data store\n");
		    goto __exit;
	    }
	    _sdata = tmp;
    }

    for (LeWeiSensor *dev = _sensors;
         dev;
         dev = static_cast<LeWeiSensor*>(dev->next))
    {
        DEBUG_PRINTS(dev->id);
        if (dev->getValue(&intval))
        {
            DEBUG_PRINTSS(" get int: ");
            Serial.println(intval);

            _sdata[i].id = dev->id;
            _sdata[i].val = (char*)malloc(8); // xxxx'\0'
            if (!_sdata[i].val)
            {
                DEBUG_PRINTSS("malloc return NULL\n");
                goto __exit;
            }
            int val_len = snprintf(_sdata[i].val, 8, "%d", intval);
            if (val_len >= 8)
            {
                DEBUG_PRINTS(dev->id);
                DEBUG_PRINTSS(" value too big: ");
                Serial.println(intval);
            }

            i++;
        }
        else if (dev->getValue(&douval))
        {
            DEBUG_PRINTSS(" get double: ");
            Serial.println(douval);

            _sdata[i].id = dev->id;
            _sdata[i].val = (char*)malloc(10); // xx.xx'\0'
            if (!_sdata[i].val)
            {
                DEBUG_PRINTSS("malloc return NULL\n");
                goto __exit;
            }
            int val_len = snprintf(_sdata[i].val, 10,
                    "%d.%02u", (int)douval, (int)(abs(douval)*100+0.5) % 100);
            if (val_len >= 10)
            {
                DEBUG_PRINTS(dev->id);
                DEBUG_PRINTSS(" value too big: ");
                Serial.println(douval);
				DEBUG_PRINTS(val_len);
            }

            i++;
        }
        else
        {
            DEBUG_PRINTSS(" no value returned\n");
        }
    }

    if (i == 0)
    {
        DEBUG_PRINTSS("no result for scanSensors\n");
        return;
    }

    send(_sdata, i);

__exit:
    for (unsigned int k = 0; k < i; k++)
    {
        free(_sdata[k].val);
    }
    free(_sdata);
    _sdata = 0;
    _sdata_len = 0;
}

int LeWeiClient::send(struct sdata *data, unsigned int i)
{
    int result = 0;

#define P(d) _client.print(d);
#define PN(d) _client.println(d);
#define PS(d) _client.print(F(d));
#define PSN(d) _client.println(F(d));
    if (_client.connect(server, 80))
    {
        // send the HTTP POST request:
        // send head.
        PS("POST /api/V1/Gateway/UpdateSensors/");
        P(_gateway);
        PS(" HTTP/1.1\r\n");
        // send userkey.
        PS("userkey: ");
        PN(_user_key);
        // send Host.
        PS("Host: open.lewei50.com \r\n");
        // send User-Agent.
        PN(user_agent);

        PS("Content-Length: ");
        int sdlen = 0;
        for (unsigned int k = 0; k < i; k++)
        {
            sdlen += strlen(data[k].id) + strlen(data[k].val);
        }
        PN(sdlen + 2 + i*23); //[] + i * ({"Name":"","Value":""},)

        // last pieces of the HTTP POST request:
        PSN("Connection: close");
        PN();

        // post data
        PS("[");
        for (unsigned int k = 0; k < i; k++)
        {
            PS("{\"Name\":\"")P(data[k].id)PS("\",\"Value\":\"")P(data[k].val)PS("\"},")
        }
        PS("]");

        result = 0;
        goto send_exit;
    }
    else
    {
        DEBUG_PRINTSS("connect failed!\n");
        result = -1;
        goto send_exit;
    }

send_exit:
#if 1
    delay(1000);
    uint8_t buf[16];
    int j;
    while((j = _client.read(buf, sizeof(buf)-1)) > 0)
    {
        buf[j] = '\0';
        DEBUG_PRINTS((char*)buf);
    }
    DEBUG_PRINTS("_client return ");
    Serial.println(j);
#endif
    _client.stop();

    return result;
#undef P
#undef PS
#undef PN
#undef PSN
}

int LeWeiClient::sendLog(char *log)
{
    int result = 0;

#define P(d) _client.print(d);
#define PN(d) _client.println(d);
#define PS(d) _client.print(F(d));
#define PSN(d) _client.println(F(d));
    if (_client.connect(server, 80))
    {
        // send head.
        PS("POST /api/V1/Gateway/UpdateLog/")P(_gateway)PS(" HTTP/1.1\r\n");
        // send userkey.
        PS("userkey: ")PN(_user_key);
        // send Host.
        PS("Host: open.lewei50.com \r\n");
        // send User-Agent.
        PN(user_agent);
        //sample json: {"Message":"网关启动成功",}
        PS("Content-Length: ")PN(strlen(log) + 15);

        // last pieces of the HTTP PUT request:
        PSN("Connection: close");
        PN();

        PS("{\"Message\":\"")P(log)PSN("\",}");

        result = 0;
    }
    else
    {
        DEBUG_PRINTSS("connect failed!\n");
        result = -1;
    }

    _client.stop();

    return result;
#undef P
#undef PN
#undef PS
#undef PSN
}

int LeWeiClient::beginServe(uint16_t port)
{
    if (_server)
        return -1;

    _server = new EthernetServer(port);
    if (!_server)
        return -2;

    _server->begin();

    return 0;
}

void LeWeiClient::dump_actuator_status_chunked(LeWeiActuator *dev)
{
#define S(d) send_chunked(d);
    S("{\"id\":\"")S(dev->id);
    S("\",\"type\":\"")S(dev->type);
    S("\",\"name\":\"")S(dev->name);
    int val;
    if (dev->getValue(&val))
    {
        char buf[10];
        snprintf(buf, sizeof(buf), "%d", val);
        S("\",\"value\":\"")S(buf);
    }
    S("\",\"status\":\"")S(dev->status)S("\"}");
#undef S
}

int LeWeiClient::on_getAllSensors(void)
{
    DEBUG_PRINTSS("got getAllSensors call\n");
#define P(d) _client.print(d);
#define PN(d) _client.println(d);
#define PS(d) _client.print(F(d));
#define PSN(d) _client.println(F(d));
#define S(d) send_chunked(d);
    // send head.
    PSN("HTTP/1.1 200 OK");
    PSN("Content-Type: text/plain");
    PSN("Transfer-Encoding: chunked");
    // last pieces of the HTTP PUT request:
    PSN("Connection: close");
    PN();

    S("{\"successful\":true,\"message\":null,\"data\":[");
    for (LeWeiDevice *dev = (LeWeiDevice*)_actuators;
         dev;
         dev = dev->next)
    {
        if (dev != (LeWeiDevice*)_actuators)
            S(",");
        dump_actuator_status_chunked((LeWeiActuator*)dev);
    }
    S("]}");
    S("");

    return 0;
#undef P
#undef PN
#undef PS
#undef PSN
#undef S
}

int LeWeiClient::on_updateSensor(char *id, char *valstr)
{
    int val = atoi((const char*)valstr);
    DEBUG_PRINTSS("updateSensor:(");
    DEBUG_PRINTS(id);
    DEBUG_PRINTSS(", ");
    DEBUG_PRINTS(valstr);
    DEBUG_PRINTSS(")\n");
#define P(d) _client.print(d);
#define PN(d) _client.println(d);
#define PS(d) _client.print(F(d));
#define PSN(d) _client.println(F(d));
#define S(d) send_chunked(d);
    // send head.
    PSN("HTTP/1.1 200 OK");
    PSN("Content-Type: text/plain");
    PSN("Transfer-Encoding: chunked");
    // last pieces of the HTTP PUT request:
    PSN("Connection: close");
    PN();

    LeWeiDevice *dev;
    for (dev = (LeWeiDevice*)_actuators;
         dev;
         dev = dev->next)
    {
        if (!strcmp(dev->id, id))
            break;
    }
    if (!dev)
    {
        S("{\"successful\":")S("false,\"message\":\"no such device\"}");
    }
    else
    {
        S("{\"successful\":")S("true,\"message\":null,\"data\":");
        ((LeWeiActuator*)dev)->updateValue(id,val);
        dump_actuator_status_chunked((LeWeiActuator*)dev);
        S("}");
    }
    S("");

    return 0;
#undef P
#undef PN
#undef PS
#undef PSN
#undef S
}

static char* check_skip(unsigned char *buf, const char *head)
{
    char *sidx = strstr((const char*)buf, head);
    if (!sidx)
    {
        DEBUG_PRINTSS("no \"");
        DEBUG_PRINTS(head);
        DEBUG_PRINTSS("\" in: ");
        DEBUG_PRINTS((const char*)buf);
        return NULL;
    }
    return sidx + strlen(head);
}

int LeWeiClient::serve(void)
{
    // don't use this in multi-threaded env
    _client = _server->available();
    if (_client) {
        DEBUG_PRINTSS("new connection come in\n");

        // let's hope there is no stack overflow here
        unsigned char buf[128];
        enum {
            HTTP_REQ,
            HTTP_HEAD,
            HTTP_BODY,
        } parser_stat = HTTP_REQ;

        for (int i = 0; _client.connected(); i++)
        {
            if (i >= sizeof(buf)-1)
            {
                DEBUG_PRINTSS("no buf to deal with request\n");
                break;
            }

            if (_client.available())
            {
                buf[i] = _client.read();
                buf[i+1] = '\0';
            }

            // only the request line is ussfull
            if (buf[i] == '\n' && parser_stat == HTTP_REQ)
            {
                if (strncmp((const char*)buf, "GET ", sizeof("GET ")-1))
                {
                    DEBUG_PRINTSS("wrong request method\n");
                    DEBUG_PRINTS((const char*)buf);
                    break;
                }

                char *uidx = check_skip(buf, "userkey=");
                if (!uidx)
                    break;

                // the http request should be greater the pure userkey
                if (strncmp(uidx, _user_key, strlen(_user_key)))
                {
                    DEBUG_PRINTSS("userkey check failed\n");
                    DEBUG_PRINTS((const char*)buf);
                    break;
                }
                DEBUG_PRINTSS("authorization success\n");

                // parse the f parameter
                char *fidx = check_skip(buf, "f=");
                if (!fidx)
                    break;

                if (!strncmp(fidx, "getAllSensors", strlen("getAllSensors")))
                {
                    on_getAllSensors();
                }
                else if (!strncmp(fidx, "updateSensor", strlen("updateSensor")))
                {
                    int t;
                    char *ididx = check_skip(buf, "p1=");
                    char *vidx = check_skip(buf, "p2=");
                    for (t = 0; ididx[t] != ' ' && ididx[t] != '&'; t++)
                        ;
                    ididx[t] = '\0';
                    for (t = 0; vidx[t] != ' ' && vidx[t] != '&'; t++)
                        ;
                    vidx[t] = '\0';
                    on_updateSensor(ididx, vidx);
                }
                else
                {
                    DEBUG_PRINTSS("un-recognized f\n");
                    DEBUG_PRINTS((const char*)buf);
                    break;
                }

                parser_stat = HTTP_HEAD;
                i = -1;
            }
            else if (parser_stat == HTTP_HEAD)
            {
                // reach the end of head
                if (strcmp((const char*)buf, "\r\n") == 0)
                {
                    parser_stat = HTTP_BODY;
                }
                else
                {
                    // skip the head
                    if (buf[i] == '\n')
                    {
                        DEBUG_PRINTSS("got http head\n");
                        DEBUG_PRINTS((const char*)buf);
                        i = -1;
                    }
                }
            }
            else if (parser_stat == HTTP_BODY)
            {
                DEBUG_PRINTSS("reach body\n");

                // give the web browser time to receive the data
                delay(1);
                break;
            }
        }
        // close the connection:
        _client.stop();
        DEBUG_PRINTSS("connection disonnected\n");
    }
}

