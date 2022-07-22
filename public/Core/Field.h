#pragma once
#include "Type.h"

namespace Core
{
	//enum EFieldFlagBits : uint32_t
	//{
	//	FFB_NoFlag = 0ULL,
	//	FFB_LValueRefBit = 1ULL << 1,
	//	FFB_RValueRefBit = 1ULL << 2,
	//	FFB_RValueRefBits = FFB_LValueRefBit | FFB_RValueRefBit,
	//};

	class MYCPP_API CLASS() Field : public Record
	{
		GENERATED_OBJECT_BODY()
	public:
		Field(const std::string& name = "", uint32_t memory_offset = 0, Type* type = nullptr)
			: Record(name)
			, _MemoryOffset(memory_offset)
			, _Type(type)
		{}

		void* GetFieldDataPtr(void const* field_owner_ptr) const { return static_cast<uint8_t*>(const_cast<void*>(field_owner_ptr)) + _MemoryOffset; }

		template<typename T>
		T* GetFieldDataPtrAs(void const* field_owner_ptr) const { return static_cast<T*>(GetFieldDataPtr(field_owner_ptr)); }

		// BoolField
		virtual void SetBool(void* field_owner_ptr, bool value) const {}
		virtual bool GetBool(void const* field_owner_ptr) const { return false; }
		virtual std::string GetBoolToString(void const* field_owner_ptr) const { return ""; }
		// NumericField
		virtual void SetUInt(void* field_owner_ptr, uint64_t value) const {}
		virtual void SetSInt(void* field_owner_ptr, int64_t value) const {}
		virtual void SetFloat(void* field_owner_ptr, double value) const {}
		virtual void SetNumericFromString(void* field_owner_ptr, char const* value) const {}
		virtual std::string GetNumericToString(void const* field_owner_ptr) const { return ""; }
		virtual int64_t GetSInt(void const* field_owner_ptr) const { return 0; }
		virtual uint64_t GetUInt(void const* field_owner_ptr) const { return 0; }
		virtual double GetFloat(void const* field_owner_ptr) const { return 0.F; }
		// StringField
		virtual std::string GetString(void const* field_owner_ptr) const { return ""; }
		virtual const char* GetCString(void const* field_owner_ptr) const { return ""; }
		virtual void SetString(void* field_owner_ptr, const std::string& value) const {}
		virtual void SetString(void* field_owner_ptr, const char* value) const {}
		virtual void SetString(void* field_owner_ptr, uint64_t value) const {}
		virtual void SetString(void* field_owner_ptr, int64_t value) const {}
		virtual void SetString(void* field_owner_ptr, double value) const {}

		uint32_t GetCastTypeFlag() const { return _Type->GetCastTypeFlag(); }
		Type* GetType() const { return _Type; }
		uint32_t GetMemoryOffset() const { return _MemoryOffset; }

	protected:
		Type* _Type{ nullptr };
		uint32_t _MemoryOffset{ 0 };
	};

	template<typename T>
	struct TFieldAccessor
	{
		enum
		{
			CppSize = sizeof(T),
			CppAlignment = alignof(T)
		};

		static T const* GetPtr(void const* A) { return (T const*)(A); }
		static T* GetPtr(void* A) { return (T*)(A); }
		static T const& Get(void const* A) { return *GetPtr(A); }

		static T GetDefault() { return T(); }
		static T GetOptional(void const* A) { return A ? Get(A) : GetDefault(); }

		static void Set(void* A, T const& value) { *GetPtr(A) = value; }

		static T* Initialize(void* A) { return new (A) T(); }
		static void Deinitialize(void* A) { GetPtr(A)->~T(); }
		static void CopyAssign(void* A, void* B) { *GetPtr(A) = *GetPtr(B); }
		static void MoveAssign(void* A, void* B) { *GetPtr(A) = std::move(*GetPtr(B)); }
	};

	template<typename T>
	class TSimpleField : public Field
	{
	public:
		TSimpleField(const std::string& name = "", uint32_t memory_offset = 0)
			: Field(name, memory_offset, Core::GetType<T>())
		{
		}
	};

	class MYCPP_API BoolField : public Field
	{
		using FieldAccessor = TFieldAccessor<bool>;
	public:
		BoolField(const std::string& name = "", uint32_t memory_offset = 0)
			: Field(name, memory_offset, Core::GetType<bool>())
		{}

		virtual void SetBool(void* field_owner_ptr, bool value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), (bool)value);
		}
		virtual bool GetBool(void const* field_owner_ptr) const override
		{
			return FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr));
		}
		virtual std::string GetBoolToString(void const* field_owner_ptr) const override
		{
			return GetBool(field_owner_ptr) ? "True" : "False";
		}
	};

	template <typename T>
	class TNumericField : public Field
	{
		using FieldAccessor = TFieldAccessor<T>;
	public:
		TNumericField(const std::string& name = "", uint32_t memory_offset = 0)
			: Field(name, memory_offset, Core::GetType<T>())
		{}

		virtual void SetUInt(void* field_owner_ptr, uint64_t value) const override
		{
			assert(std::is_integral_v<T>);
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), (T)value);
		}
		virtual void SetSInt(void* field_owner_ptr, int64_t value) const override
		{
			assert(std::is_integral_v<T>);
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), (T)value);
		}
		virtual void SetFloat(void* field_owner_ptr, double value) const override
		{
			assert(std::is_floating_point_v<T>);
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), (T)value);
		}
		virtual void SetNumericFromString(void* field_owner_ptr, char const* value) const override
		{
			*FieldAccessor::GetPtr(Field::GetFieldDataPtr(field_owner_ptr)) = (T)atof(value);
		}
		virtual std::string GetNumericToString(void const* field_owner_ptr) const override
		{
			return std::to_string(FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr)));
		}
		virtual int64_t GetSInt(void const* field_owner_ptr) const override
		{
			assert(std::is_integral_v<T>);
			return (int64_t)FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr));
		}
		virtual uint64_t GetUInt(void const* field_owner_ptr) const override
		{
			assert(std::is_integral_v<T>);
			return (uint64_t)FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr));
		}
		virtual double GetFloat(void const* field_owner_ptr) const override
		{
			assert(std::is_floating_point_v<T>);
			return (double)FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr));
		}
	};

	MYCPP_API template class TNumericField<int8_t>;
	MYCPP_API template class TNumericField<int16_t>;
	MYCPP_API template class TNumericField<int32_t>;
	MYCPP_API template class TNumericField<int64_t>;
	MYCPP_API template class TNumericField<int8_t>;
	MYCPP_API template class TNumericField<int16_t>;
	MYCPP_API template class TNumericField<int32_t>;
	MYCPP_API template class TNumericField<int64_t>;
	MYCPP_API template class TNumericField<float>;
	MYCPP_API template class TNumericField<double>;

	using SInt8Field  = TNumericField<int8_t>;
	using SInt16Field = TNumericField<int16_t>;
	using SInt32Field = TNumericField<int32_t>;
	using SInt64Field = TNumericField<int64_t>;
	using UInt8Field  = TNumericField<int8_t>;
	using UInt16Field = TNumericField<int16_t>;
	using UInt32Field = TNumericField<int32_t>;
	using UInt64Field = TNumericField<int64_t>;
	using FloatField  = TNumericField<float>;
	using DoubleField = TNumericField<double>;


	class MYCPP_API StringField : public  Field
	{
		using FieldAccessor = TFieldAccessor<std::string>;
	public:

		StringField(const std::string& name = "", uint32_t memory_offset = 0)
			: Field(name, memory_offset, Core::GetType<std::string>())
		{}

		virtual std::string GetString(void const* field_owner_ptr) const override
		{
			return FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr));
		}

		virtual const char* GetCString(void const* field_owner_ptr) const override
		{
			return FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr)).c_str();
		}

		virtual void SetString(void* field_owner_ptr, const std::string& value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), value);
		}

		virtual void SetString(void* field_owner_ptr, const char* value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), value);
		}

		virtual void SetString(void* field_owner_ptr, uint64_t value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), std::to_string(value));
		}

		virtual void SetString(void* field_owner_ptr, int64_t value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), std::to_string(value));
		}

		virtual void SetString(void* field_owner_ptr, double value) const override
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), std::to_string(value));
		}
	};

	class MYCPP_API ClassField : public Field
	{
	public:
		ClassField(const std::string& name = "", uint32_t memory_offset = 0, Type* type = nullptr)
			: Field(name, memory_offset, type)
		{}
	};

	template<typename T>
	class TClassField : public ClassField
	{
	public:
		TClassField(const std::string& name = "", uint32_t memory_offset = 0)
			: ClassField(name, memory_offset, Core::GetType<T>())
		{}

	};

	class MYCPP_API EnumField : public Field
	{
	public:
		EnumField(const std::string& name = "", uint32_t memory_offset = 0, Type* type = nullptr)
			: Field(name, memory_offset, type)
		{}
	};

	template<typename T>
	class TEnumField : public EnumField
	{
		using FieldAccessor = TFieldAccessor<T>;
	public:
		TEnumField(const std::string& name = "", uint32_t memory_offset = 0)
			: EnumField(name, memory_offset, Core::GetType<T>())
		{}
		virtual void SetUInt(void* field_owner_ptr, uint64_t value) const override 
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), static_cast<T>(value));
		}
		virtual void SetSInt(void* field_owner_ptr, int64_t value) const override 
		{
			FieldAccessor::Set(Field::GetFieldDataPtr(field_owner_ptr), static_cast<T>(value));
		}
		virtual int64_t GetSInt(void const* field_owner_ptr) const override 
		{
			return static_cast<int64_t>(FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr)));
		}
		virtual uint64_t GetUInt(void const* field_owner_ptr) const override 
		{ 
			return static_cast<uint64_t>(FieldAccessor::Get(Field::GetFieldDataPtr(field_owner_ptr)));
		}
	};

	template<typename T>
	class MYCPP_API TPointerField : public Field
	{
	public:
		TPointerField(const std::string& name = "", uint32_t memory_offset = 0, Type* type = nullptr)
			: Field(name, memory_offset, Core::GetType<T>())
		{}
	};

	template<typename T>
	class TReferenceWrapperField : public Field
	{
	public:
		TReferenceWrapperField(const std::string& name = "", uint32_t memory_offset = 0, Type* type = nullptr)
			: Field(name, memory_offset, Core::GetType<T>())
		{}
	};

	template<typename T>
	std::unique_ptr<Field> MakeField(const std::string& name, uint32_t memory_offset)
	{
		static_assert(!std::is_reference_v<T>);
		Field* Field = nullptr;
		if constexpr (IsReferenceWrapperType<T>::value)
			return std::make_unique<TReferenceWrapperField<T>>(name, memory_offset);
		else if constexpr (std::is_pointer_v<T>)
			return std::make_unique<TPointerField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<bool, T>)
			return std::make_unique<BoolField>(name, memory_offset);
		else if constexpr (std::is_arithmetic_v<T>)
			return std::make_unique<TNumericField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::string>)
			return std::make_unique<StringField>(name, memory_offset);
		else if constexpr (IsReflectClassType<T>::value)
			return std::make_unique<TClassField<T>>(name, memory_offset);
		else if constexpr (std::is_enum_v<T>)
			return std::make_unique<TEnumField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::vector<std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::map<void*, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::map<int64_t, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::map<std::string, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::unordered_map<void*, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::unordered_map<int64_t, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::any>>)
			return std::make_unique<TSimpleField<T>>(name, memory_offset);
		else
			static_assert(ConstexprFalse<T> && "UNSUPPORTED TYPE");
		return nullptr;
	}

	template<typename T>
	std::unique_ptr<Field> MakeFieldByConvertToRefWrapper(const std::string& name, uint32_t memory_offset)
	{
		return MakeField<ConvertToRefWrapper<T>>(name, memory_offset);
	}
	
}




