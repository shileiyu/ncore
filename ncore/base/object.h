#ifndef NCORE_BASE_BASEOBJECT_H_
#define NCORE_BASE_BASEOBJECT_H_

#include <ncore/ncore.h>   
#include "atomic.h"

namespace ncore
{

class NonCopyableObject
{
protected:
    NonCopyableObject() {};
private:
    //disallow cpoy and assign
    NonCopyableObject(const NonCopyableObject &);
    NonCopyableObject & operator=(const NonCopyableObject &);
};

}

#endif