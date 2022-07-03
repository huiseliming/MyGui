#include "Core/Class.h"

namespace Core
{
	// @test begin
	void TCustomTypeModifier<TestStruct>::operator()(Type* initialized_type)
	{
		Class* initialized_class = Cast<Class>(initialized_type);
		if (initialized_class)
		{
			initialized_class->_Fields.push_back(MakeField<bool>("manual_field", offsetof(TestStruct, _Boolean)));
		}
	}
	// @test end
}