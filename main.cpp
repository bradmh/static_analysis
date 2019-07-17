#include <iostream>
#include <memory>
#include <cstdarg>

namespace helper
{
	void zero_ptr(char*& p) noexcept
	{
		p = nullptr;
	}

	template<typename ptr_type, typename value_type>
	void write_through(ptr_type ptr, value_type val) noexcept
	{
		*ptr = val;
	}

	auto make_pointer_knackering_functor(char*& p) noexcept
	{
		return [&p]() noexcept { zero_ptr(p); };
	}

	void conditional_chuck_exception(const char* to_test)
	{
		if (to_test && *to_test == 'p') throw "We're all doomed...";
	}

	char* address_of_out_of_scope_local_variable() noexcept
	{
		char buffer[1024]{};
		auto p{ &buffer[0] };
		return p;
	}

	char* address_of_out_of_scope_stack_space() noexcept
	{
		auto p{ alloca(1024) };
		return static_cast<char*>( p );
	}

	int var_arg_function(int dummy, ...) noexcept
	{
		va_list arg_list;
		va_start(arg_list, dummy);

		const int n1 = va_arg(arg_list, int);
		const int n2 = va_arg(arg_list, int);
		const int n3 = va_arg(arg_list, int);
		const int n4 = va_arg(arg_list, int);

		va_end(arg_list);

		return n1 + n2 + n3 + n4;
	}

	void overloaded_reporting_test_function( const std::string & )
	{
		std::cout << "std::string overload called" << std::endl;
	}

	void overloaded_reporting_test_function( bool )
	{
		std::cout << "bool overload called" << std::endl;
	}

	char* dynamic_buffer()
	{
		return new char[1024]{};
	}

	class program_crasher
	{
		public:
			~program_crasher()
			{
				throw std::runtime_error( "We're going down in flames Captain!" );
			}
	};

	class base_exception
	{
		public:
			int error_;
	};

	class derived_exception : public base_exception
	{
		public:
			int extended_;
	};

	void throw_derived()
	{
		throw derived_exception{};
	}
}

namespace tests
{
	void dereference_pointer_zeroed_inline() noexcept
	{
		char* p{ nullptr };
		*p = 'p';
	}

	void dereference_null_pointer_passed_as_argument() noexcept
	{
		helper::write_through( static_cast<int *>( nullptr ), 1 );
	}

	void dereference_pointer_zeroed_by_function() noexcept
	{
		char buffer[1024]{};
		auto p{ &buffer[0] };
		helper::zero_ptr(p);
		helper::write_through(p, 'p');
	}

	void dereference_pointer_zeroed_by_functor() noexcept
	{
		char buffer[1024]{};
		auto p{ &buffer[0] };
		const auto f{ helper::make_pointer_knackering_functor(p) };
		f();
		helper::write_through(p, 'p');
	}

	void leak_memory_by_exiting_function_after_new()
	{
		auto p{ helper::dynamic_buffer() };
		helper::write_through(p, 'p');
	}

	void leak_memory_by_releasing_smart_pointer()
	{
		auto p{ std::make_unique<char[]>(1024) };
		helper::write_through(p.get(), 'p');
		p.release();
	}

	void leak_memory_by_throwing_after_new()
	{
		auto p{ helper::dynamic_buffer() };
		helper::write_through(p, 'p');
		helper::conditional_chuck_exception(p);
	}
	
	void delete_object_with_incorrect_delete()
	{
		auto p{ helper::dynamic_buffer() };
		helper::write_through(p, 'p');
		delete p;
	}

	void return_local_variable_address() noexcept
	{
		auto p{ helper::address_of_out_of_scope_local_variable() };
		helper::write_through(p, 'p');
	}

	void return_address_of_block_allocated_from_the_stack()
	{
		auto p{ helper::address_of_out_of_scope_stack_space() };
		helper::write_through(p, 'p');
	}

	void send_too_many_arguments_to_printf_family_function_using_constant_for_format() noexcept
	{
		std::printf( "%s", "Urgle", "Bargle");
	}

	void send_too_many_arguments_to_printf_family_function_using_variable_for_format()
	{
		std::string format{ "%s" };
		std::printf( format.c_str(), "Urgle", "Bargle" );
	}

	void send_aguments_inconsistant_with_format_to_printf_family_function() noexcept
	{
		int a{};
		std::printf("%s %d", a, "Bargle");
	}

	void send_too_few_arguments_to_scanf_family_function_using_constant_for_format() noexcept
	{
		int a{}, b{};
		std::scanf( "%d %d %d", &a, &b );
	}

	void send_too_few_arguments_to_scanf_family_function_using_variable_for_format()
	{
		std::string format{ "%d %d %d" };
		int a{}, b{};
		std::scanf(format.c_str(), &a, &b);
	}

	void not_supplying_enough_arguments_to_variable_parameter_function()
	{
		std::cout << helper::var_arg_function(0, 1);
	}

	void modifying_a_const_object_through_non_const_alias() noexcept
	{
		const char n{ '-' };
		helper::write_through(const_cast<char*>(&n), 'Q');
	}

	void call_overload_requiring_trivial_argument_conversion()
	{
		helper::overloaded_reporting_test_function( "Urgle");
	}

	void indirect_throw_from_a_destructor() noexcept
	{
		helper::program_crasher banzai;
	}

	void throw_from_no_except_function() noexcept
	{
		throw 97;
	}

	void incorrect_destructor_called_by_base_class_not_having_virtual_destructor()
	{
		class base {};
		class derived : public base {};

		base* p{ new derived() };
		delete p;
	}

	void incorrect_destructor_called_by_assigning_base_class_smart_pointer()
	{
		class base {};
		class derived : public base {};

		base* p{ new derived() };
		std::unique_ptr<base> thicky_not_smart{ p };
	}

	void slicing_exception_by_catching_base_class_by_value()
	try
	{
		helper::throw_derived();
		std::cout << "We shouldn't get here!" << std::endl;
	}
	catch (helper::base_exception e)
	{
		std::cout << "Caught base_exception!" << e.error_ << std::endl;
	}

	void converting_pointers_to_incompatible_types_through_void()
	{
		class A { public: int value_; };
		class B { public: float value_; };

		A a{};
		void *p{ &a };
		auto q{ static_cast<B*>(p) };
		std::cout << q->value_ << std::endl;
	}

	void narrowing_conversions_during_initialisation()
	{
		int biblical_pi( 3.14159265 );
		std::cout << "According to the word of God, Pi is: "
				  << biblical_pi
				  << " Which just shows that the code for the universe was written in C++2003"
				  << std::endl;
	}
}

int main()
try
{
	using namespace tests;

	dereference_pointer_zeroed_inline();
	dereference_null_pointer_passed_as_argument();
	dereference_pointer_zeroed_by_function();
	dereference_pointer_zeroed_by_functor();
	leak_memory_by_exiting_function_after_new();
	leak_memory_by_releasing_smart_pointer();
	leak_memory_by_throwing_after_new();
	delete_object_with_incorrect_delete();
	return_local_variable_address();
	return_address_of_block_allocated_from_the_stack();
	send_too_many_arguments_to_printf_family_function_using_constant_for_format();
	send_too_many_arguments_to_printf_family_function_using_variable_for_format();
	send_aguments_inconsistant_with_format_to_printf_family_function();
	send_too_few_arguments_to_scanf_family_function_using_constant_for_format();
	send_too_few_arguments_to_scanf_family_function_using_variable_for_format();
	not_supplying_enough_arguments_to_variable_parameter_function();
	modifying_a_const_object_through_non_const_alias();
	call_overload_requiring_trivial_argument_conversion();
	indirect_throw_from_a_destructor();
	throw_from_no_except_function();
	incorrect_destructor_called_by_base_class_not_having_virtual_destructor();
	incorrect_destructor_called_by_assigning_base_class_smart_pointer();
	slicing_exception_by_catching_base_class_by_value();
	converting_pointers_to_incompatible_types_through_void();
	narrowing_conversions_during_initialisation();

	std::cout << "If you can see this, there's something wrong!" << std::endl;
}
catch (const std::exception& e)
{
	std::cerr << "Something went wrong: " << e.what() << std::endl;
}
catch (...)
{
	std::cerr << "Something went wrong: No idea what!" << std::endl;
}
