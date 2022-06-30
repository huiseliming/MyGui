#include "Reflect/Class.h"

namespace Reflect
{
	// @test begin
	void TCustomClassModifier<TestStruct>::operator()(Class* uninitialized_class)
	{
		uninitialized_class->_Fields.push_back(MakeField<bool>("Boolean", offsetof(TestStruct, _Boolean)));
	}
	// @test end
}