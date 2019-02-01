#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <time.h>
#include "Synchronizer.h"
#include "common.h"

struct PluginCfg;

//+------------------------------------------------------------------+
//| Simple configuration                                             |
//+------------------------------------------------------------------+
class Config {
private:
    Synchronizer m_sync;        // synchronizer
    char m_filename[MAX_PATH];  // name of the configuration file
    PluginCfg* m_cfg;           // configs
    int m_cfg_total;            // total number of records
    int m_cfg_max;              // max number of records

public:
    static Config& Instance();

    //--- Initializing the database (reading the config file)
    void Load(const char* filename);
    void Save(void);

    //--- access
    int Add(const char* name, const char* value, bool save = false);
    int Add(const PluginCfg* cfg);
    int Set(const PluginCfg* values, const int total);
    int Get(const char* name, PluginCfg* cfg);
    int Next(const int index, PluginCfg* cfg);
    int Delete(const char* name);
    inline int Total(void) {
        m_sync.Lock();
        int total = m_cfg_total;
        m_sync.Unlock();
        return (total);
    }

    int GetInteger(const char* name, int* value, const char* defvalue = NULL);
    int GetLong(const char* name, long* value, const char* defvalue = NULL);
    int GetString(const char* name, char* value, const int maxlen, const char* defvalue = NULL);
    bool HasKey(const char* name);

private:
    Config();
    ~Config();
    Config(Config const&) {}
    void operator=(Config const&) {}
    PluginCfg* Search(const char* name);
    static int SortByName(const void* left, const void* right);
    static int SearchByName(const void* left, const void* right);
};

// extern Config ExtConfig;
//+------------------------------------------------------------------+
#endif  // !_CONFIGURATION_H_
