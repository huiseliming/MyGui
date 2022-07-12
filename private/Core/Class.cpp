#include "Core/Class.h"

void TCustomTypeModifier<Core::TestStruct>::operator()(Core::Type* initialized_type)
{
	using namespace Core;
	Class* initialized_class = Cast<Class>(initialized_type);
	if (initialized_class)
	{
		initialized_class->_Fields.push_back(MakeField<bool>("manual_field", offsetof(Core::TestStruct, _Boolean)));
	}
}