#pragma once
#include "Struct.h"

namespace Reflect
{

	class MYGUI_API Class : public Struct
	{
	public:
		Class(const std::string& name = "")
			: Struct(name)
		{}


	private:
		template<typename T> friend struct TCustomReflectModifier;
		template<typename T> friend struct TDefaultReflectInitializer;
	};


	template<typename T>
	struct TCustomReflectModifier {
		void operator()(Class* initialized_class) {}
	};
	template<typename T>
	struct TDefaultReflectInitializer {
		void operator()(Class* uninitialized_class) {}
	};

	template<typename T>
	struct TReflectClassAutoInitializer {
		TReflectClassAutoInitializer() {
			Class* reflect_class = T::StaticClass();
			TDefaultReflectInitializer<T>()(reflect_class);
			TCustomReflectModifier<T>()(reflect_class);
		}
	};
}
















