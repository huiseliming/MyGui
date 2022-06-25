#pragma once
#include "Type.h"
#include "Field.h"

namespace Reflect
{
	class Struct : public Type
	{
	public:
		Struct(const std::string& name = "") : Type(name) {}

		const std::vector<std::unique_ptr<Field>>& GetFields() { return _Fields; }

	protected:
		std::vector<std::unique_ptr<Field>> _Fields;

	};
}