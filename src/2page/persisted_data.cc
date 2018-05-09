#include "persisted_data.h"
#include "1mem/mem.h"


namespace upscaledb {

PersistedData::PersistedData()
    : address( 0 )
    , size( 0 )
    , is_dirty( false )
    , is_allocated( false )
    , is_without_header( false )
    , raw_data( nullptr )
{
}


PersistedData::~PersistedData()
{
#ifdef NDEBUG
    mutex.safe_unlock();
#endif
    if( is_allocated )
    {
        Memory::release( raw_data );
    }
    raw_data = nullptr;
}



}
