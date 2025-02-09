
#pragma once

#include "demhandlers.h"
#include "base/array.h"
#include <string>
#include <vector>

#define PROPINFOBITS_NUMPROPS           10
#define PROPINFOBITS_NUMPROPS           10
#define PROPINFOBITS_TYPE               5
#define PROPINFOBITS_FLAGS              SPROP_NUMFLAGBITS_NETWORKED
#define PROPINFOBITS_STRINGBUFFERLEN    10
#define PROPINFOBITS_NUMBITS            7
#define PROPINFOBITS_RIGHTSHIFT         6
#define PROPINFOBITS_NUMELEMENTS        10 // For arrays.


// SendProp::m_Flags.
#define SPROP_UNSIGNED			(1<<0)	// Unsigned integer data.

#define SPROP_COORD				(1<<1)	// If this is set, the float/vector is treated like a world coordinate.
										// Note that the bit count is ignored in this case.

#define SPROP_NOSCALE			(1<<2)	// For floating point, don't scale into range, just take value as is.

#define SPROP_ROUNDDOWN			(1<<3)	// For floating point, limit high value to range minus one bit unit

#define SPROP_ROUNDUP			(1<<4)	// For floating point, limit low value to range minus one bit unit

#define SPROP_NORMAL			(1<<5)	// If this is set, the vector is treated like a normal (only valid for vectors)
							
#define SPROP_EXCLUDE			(1<<6)	// This is an exclude prop (not excludED, but it points at another prop to be excluded).

#define SPROP_XYZE				(1<<7)	// Use XYZ/Exponent encoding for vectors.

#define SPROP_INSIDEARRAY		(1<<8)	// This tells us that the property is inside an array, so it shouldn't be put into the
										// flattened property list. Its array will point at it when it needs to.

#define SPROP_PROXY_ALWAYS_YES	(1<<9)	// Set for datatable props using one of the default datatable proxies like
										// SendProxy_DataTableToDataTable that always send the data to all clients.

#define SPROP_CHANGES_OFTEN		(1<<10)	// this is an often changed field, moved to head of sendtable so it gets a small index

#define SPROP_IS_A_VECTOR_ELEM	(1<<11)	// Set automatically if SPROP_VECTORELEM is used.

#define SPROP_COLLAPSIBLE		(1<<12)	// Set automatically if it's a datatable with an offset of 0 that doesn't change the pointer
										// (ie: for all automatically-chained base classes).
										// In this case, it can get rid of this SendPropDataTable altogether and spare the
										// trouble of walking the hierarchy more than necessary.

#define SPROP_COORD_MP					(1<<13) // Like SPROP_COORD, but special handling for multiplayer games
#define SPROP_COORD_MP_LOWPRECISION 	(1<<14) // Like SPROP_COORD, but special handling for multiplayer games where the fractional component only gets a 3 bits instead of 5
#define SPROP_COORD_MP_INTEGRAL			(1<<15) // SPROP_COORD_MP, but coordinates are rounded to integral boundaries

#define SPROP_VARINT					SPROP_NORMAL	// reuse existing flag so we don't break demo. note you want to include SPROP_UNSIGNED if needed, its more efficient

#define SPROP_NUMFLAGBITS_NETWORKED		16

// This is server side only, it's used to mark properties whose SendProxy_* functions encode against gpGlobals->tickcount (the only ones that currently do this are
//  m_flAnimTime and m_flSimulationTime.  MODs shouldn't need to mess with this probably
#define SPROP_ENCODED_AGAINST_TICKCOUNT	(1<<16)

// See SPROP_NUMFLAGBITS_NETWORKED for the ones which are networked
#define SPROP_NUMFLAGBITS				17


enum
{
    DPT_Int=0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_Int64,
    DPT_NUMSendPropTypes
};

namespace DemMsg
{
    struct Dem_DataTables
    {
        static const int DATA_MAX_LENGTH = 256 * 1024;

        struct SendProp
        {
            std::string name;
            std::string exclude;
            int type;
            int flags;
            int elements;
            int lowValue;
            int highValue;
            int bits;
        };

        struct SendTable
        {
            bool needsDecoder;
            std::string name;
            int numProps;
            std::vector<SendProp> props;
        };
        std::vector<SendTable> sendtables;

        struct DataClass
        {
            std::string className;
            std::string datatableName;
        };
        Array<DataClass> classes;
    };
}

DECLARE_DEM_HANDLERS(Dem_DataTables);
