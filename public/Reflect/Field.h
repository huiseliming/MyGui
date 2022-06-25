#pragma once
#include "Type.h"

namespace Reflect
{
	class Field : public Record
	{
	public:
		Field(const std::string& name = "", uint32_t memory_offset = 0)
			: Record(name)
			, _MemoryOffset(memory_offset)
		{}
		template<typename T = void>
		T* GetStructFieldPtr(void const* struct_owner_ptr) const { return static_cast<T*>(static_cast<uint8_t*>(struct_owner_ptr) + _MemoryOffset); }

		// BoolField
		virtual void SetBool(void* struct_owner_ptr, bool value) const {}
		virtual bool GetBool(void const* struct_owner_ptr) const { return false; }
		virtual std::string GetBoolToString(void const* struct_owner_ptr) const { return ""; }
		// NumericField
		virtual void SetUInt(void* struct_owner_ptr, uint64_t value) const {}
		virtual void SetSInt(void* struct_owner_ptr, int64_t value) const {}
		virtual void SetFloat(void* struct_owner_ptr, double value) const {}
		virtual void SetNumericFromString(void* struct_owner_ptr, char const* value) const {}
		virtual std::string GetNumericToString(void const* struct_owner_ptr) const { return ""; }
		virtual int64_t GetSInt(void const* struct_owner_ptr) const { return 0; }
		virtual uint64_t GetUInt(void const* struct_owner_ptr) const { return 0; }
		virtual double GetFloat(void const* struct_owner_ptr) const { return 0.F; }
		// StringField
		virtual std::string GetString(void const* struct_owner_ptr) const { return ""; }
		virtual const char* GetCString(void const* struct_owner_ptr) const { return ""; }
		virtual void SetString(void* struct_owner_ptr, const std::string& value) const {}
		virtual void SetString(void* struct_owner_ptr, const char* value) const {}
		virtual void SetString(void* struct_owner_ptr, uint64_t value) const {}
		virtual void SetString(void* struct_owner_ptr, int64_t value) const {}
		virtual void SetString(void* struct_owner_ptr, double value) const {}

	protected:
		Type* _Type;
		uint32_t _MemoryOffset;
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
	class TField : public Field
	{
	public:
		TField(const std::string& name = "", uint32_t memory_offset = 0)
			: Field(name, memory_offset)
		{
			_Type = GetReflType<T>();
		}
	};

	class BoolField : public TField<bool>
	{
		using FieldAccessor = TFieldAccessor<bool>;
	public:
		BoolField(const std::string& name = "", uint32_t memory_offset = 0)
			: TField<bool>(name, memory_offset)
		{}

		virtual void SetBool(void* struct_owner_ptr, bool value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), (bool)value);
		}
		virtual bool GetBool(void const* struct_owner_ptr) const override
		{
			return FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr));
		}
		virtual std::string GetBoolToString(void const* struct_owner_ptr) const
		{
			return GetBool(struct_owner_ptr) ? "True" : "False";
		}
	};

	template <typename T>
	class TNumericField : public TField<T>
	{
		using FieldAccessor = TFieldAccessor<T>;
	public:
		TNumericField(const std::string& name = "", uint32_t memory_offset = 0)
			: TField<T>(name, memory_offset)
		{}

		virtual void SetUInt(void* struct_owner_ptr, uint64_t value) const override
		{
			assert(std::is_integral_v<T>);
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), (T)value);
		}
		virtual void SetSInt(void* struct_owner_ptr, int64_t value) const override
		{
			assert(std::is_integral_v<T>);
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), (T)value);
		}
		virtual void SetFloat(void* struct_owner_ptr, double value) const override
		{
			assert(std::is_floating_point_v<T>);
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), (T)value);
		}
		virtual void SetNumericFromString(void* struct_owner_ptr, char const* value) const override
		{
			*FieldAccessor::GetPtr(GetStructFieldPtr(struct_owner_ptr)) = (T)atof(value);
		}
		virtual std::string GetNumericToString(void const* struct_owner_ptr) const override
		{
			return std::to_string(FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr)));
		}
		virtual int64_t GetSInt(void const* struct_owner_ptr) const override
		{
			assert(std::is_integral_v<T>);
			return (int64_t)FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr));
		}
		virtual uint64_t GetUInt(void const* struct_owner_ptr) const override
		{
			assert(std::is_integral_v<T>);
			return (uint64_t)FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr));
		}
		virtual double GetFloat(void const* struct_owner_ptr) const override
		{
			assert(std::is_floating_point_v<T>);
			return (double)FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr));
		}
	};

	template class TNumericField<int8_t>;
	template class TNumericField<int16_t>;
	template class TNumericField<int32_t>;
	template class TNumericField<int64_t>;
	template class TNumericField<int8_t>;
	template class TNumericField<int16_t>;
	template class TNumericField<int32_t>;
	template class TNumericField<int64_t>;
	template class TNumericField<float>;
	template class TNumericField<double>;

	using SInt8Field = TNumericField<int8_t>;
	using SInt16Field = TNumericField<int16_t>;
	using SInt32Field = TNumericField<int32_t>;
	using SInt64Field = TNumericField<int64_t>;
	using UInt8Field = TNumericField<int8_t>;
	using UInt16Field = TNumericField<int16_t>;
	using UInt32Field = TNumericField<int32_t>;
	using UInt64Field = TNumericField<int64_t>;
	using FloatField = TNumericField<float>;
	using DoubleField = TNumericField<double>;


	class StringField : public  TField<std::string>
	{
		using FieldAccessor = TFieldAccessor<std::string>;
	public:

		StringField(const std::string& name = "", uint32_t memory_offset = 0)
			: TField<std::string>(name, memory_offset)
		{}

		virtual std::string GetString(void const* struct_owner_ptr) const override
		{
			return FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr));
		}

		virtual const char* GetCString(void const* struct_owner_ptr) const override
		{
			return FieldAccessor::Get(GetStructFieldPtr(struct_owner_ptr)).c_str();
		}

		virtual void SetString(void* struct_owner_ptr, const std::string& value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), value);
		}

		virtual void SetString(void* struct_owner_ptr, const char* value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), value);
		}

		virtual void SetString(void* struct_owner_ptr, uint64_t value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), std::to_string(value));
		}

		virtual void SetString(void* struct_owner_ptr, int64_t value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), std::to_string(value));
		}

		virtual void SetString(void* struct_owner_ptr, double value) const override
		{
			FieldAccessor::Set(GetStructFieldPtr(struct_owner_ptr), std::to_string(value));
		}
	};
	
	template<typename T>
	class TClassField : public TField<T>
	{
	public:
		TClassField(const std::string& name = "", uint32_t memory_offset = 0)
			: TField<T>(name, memory_offset)
		{}
	};

	template<typename T>
	std::unique_ptr<Field> MakeField(const std::string& name, uint32_t memory_offset)
	{
		static_assert(!std::is_reference_v<T>);
		Field* Field = nullptr;
		if      constexpr (std::is_same_v<bool, T>) Field = new BoolField(InName, InOffset);
		else if constexpr (std::is_arithmetic_v<T>) Field = new TNumericField<T>(InName, InOffset);
		else if constexpr (std::is_same_v<T, std::string>) Field = new StringField(InName, InOffset);
		else if constexpr (HasStaticClass<T>::value) Field = new TClassField(InName, InOffset);
		else if constexpr (std::is_enum_v<T>) Field = new TEnumField<T>(InName, InOffset);
		else    static_assert(NewFieldNotSupported<T> && "UNSUPPORTED TYPE");
		return Field;
	}


	template<typename T>
	std::unique_ptr<Field> MakeRefField(const std::string& name, uint32_t memory_offset)
	{
		static_assert(std::is_reference_v<T>);
		return Field;
	}
	
	
}




