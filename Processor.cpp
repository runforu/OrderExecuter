#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

void Processor::Shutdown() {}

void Processor::Initialize() {
    m_api.UnitTest();
}
