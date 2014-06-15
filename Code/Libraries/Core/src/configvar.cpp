#include "core.h"
#include "configvar.h"

ConfigVar::ConfigVar()
    : m_Type(EVT_None),
      m_Bool(false),
      m_Int(0),
      m_Float(0),
      m_String(NULL),
      m_Hash((uint)0)
#if PARANOID_HASH_CHECK
      ,
      m_Name("")
#endif
{
}