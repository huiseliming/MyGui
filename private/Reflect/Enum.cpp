#include "Reflect/Enum.h"

namespace Reflect
{
	EnumValue EmptyEnumValue;

    void TCustomEnumModifier<ETestEnum>::operator()(Enum* initialized_enum) 
	{
		initialized_enum->_EnumValueMap.insert(std::make_pair(ETestEnum::TE_6, EnumValue{ ETestEnum::TE_6, "ETestEnum::TE_6", "ETestEnum::TE_6666" }));
	}
}

