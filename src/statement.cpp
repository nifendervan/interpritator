#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

bool was_return = false;

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
//	cout << "Assignment: " << var_name << endl;
//	cout << "Closure: ";
//	for (const auto& [k, v] : closure) {
//		cout << k << ", ";
//	}
//	cout << endl;

	closure[var_name] = right_value->Execute(closure);
	return closure[var_name];
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv)
	: var_name(move(var)),
		right_value(move(rv)) {}

VariableValue::VariableValue(std::string var_name) {
	dotted_ids.push_back(move(var_name));
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids)
	: dotted_ids(move(dotted_ids)) {}

ObjectHolder VariableValue::Execute(Closure& closure) {
//	cout << "Closure: ";
//	for (const auto& [k, v] : closure) {
//		cout << k << ", ";
//	}
//	cout << endl;

	Closure temp_closure = closure;

	ObjectHolder object;
	for (const auto& id : dotted_ids) {
//		cout << id << endl;
		if (temp_closure.count(id) == 0) {
			throw runtime_error("Not found: " + id);
		}
		object = temp_closure[id];
		if (object.TryAs<Runtime::ClassInstance>()) {
			temp_closure = object.TryAs<Runtime::ClassInstance>()->Fields();
		}
	}

	return object;
}

unique_ptr<Print> Print::Variable(std::string var) {
	return make_unique<Print>(make_unique<VariableValue>(move(var)));
}

Print::Print(unique_ptr<Statement> argument) {
	args.push_back(move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args)
	: args(move(args)) {}

ObjectHolder Print::Execute(Closure& closure) {
//	output = &cout;
	bool first = true;
	for (auto& arg : args) {
		if (!first) {
			*output << ' ';
		}
		first = false;

		arg = make_unique<Stringify>(move(arg));
		arg->Execute(closure)->Print(*output);
//		*output << endl;
	}
	*output << '\n';
	return ObjectHolder::None();
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
  std::unique_ptr<Statement> object,
	std::string method,
	std::vector<std::unique_ptr<Statement>> args)
	: object(move(object)),
		method(move(method)),
		args(move(args)) {}

ObjectHolder MethodCall::Execute(Closure& closure) {
	vector<ObjectHolder> actual_args;
	actual_args.reserve(args.size());
	for (const auto& arg : args) {
		actual_args.push_back(arg->Execute(closure));
	}

	was_return = false;
	ObjectHolder holder = object->Execute(closure).TryAs<Runtime::ClassInstance>()->Call(method, actual_args);
	was_return = false;
	return holder;
}

ObjectHolder Stringify::Execute(Closure& closure) {
	ObjectHolder object = argument->Execute(closure);
	ostringstream out;
	if (object) {
		object->Print(out);
	} else {
		out << "None";
	}
	return ObjectHolder::Own(Runtime::String(out.str()));
}

ObjectHolder Add::Execute(Closure& closure) {
	using Runtime::Number;
	using Runtime::String;
	using Runtime::ClassInstance;

	ObjectHolder left = lhs->Execute(closure);
	ObjectHolder right = rhs->Execute(closure);

	if (!left || !right) {
		throw runtime_error("");
	}

	if (left.TryAs<ClassInstance>()) {
		ClassInstance* object = left.TryAs<ClassInstance>();
		if (object->HasMethod("__add__", 1)) {
			return object->Call("__add__", {right});
		} else {
			throw runtime_error("");
		}
	}

	if (right.TryAs<ClassInstance>()) {
		ClassInstance* object = right.TryAs<ClassInstance>();
		if (object->HasMethod("__add__", 1)) {
			return object->Call("__add__", {left});
		} else {
			throw runtime_error("");
		}
	}

	if (left.TryAs<Number>()) {
		if (!right.TryAs<Number>()) {
			throw runtime_error("");
		}
		int l = left.TryAs<Number>()->GetValue();
		int r = right.TryAs<Number>()->GetValue();
		return ObjectHolder::Own(Number(l + r));
	}

	if (left.TryAs<String>()) {
		if (!right.TryAs<String>()) {
			throw runtime_error("");
		}
		string l = left.TryAs<String>()->GetValue();
		string r = right.TryAs<String>()->GetValue();
//		cout << (l + r) << endl;
		return ObjectHolder::Own(String(l + r));
	}

	throw runtime_error("");
}

ObjectHolder Sub::Execute(Closure& closure) {
	using Runtime::Number;

	int left = lhs->Execute(closure).TryAs<Number>()->GetValue();
	int right = rhs->Execute(closure).TryAs<Number>()->GetValue();
	return ObjectHolder::Own(Number(left - right));
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
	using Runtime::Number;

	int left = lhs->Execute(closure).TryAs<Number>()->GetValue();
	int right = rhs->Execute(closure).TryAs<Number>()->GetValue();
	return ObjectHolder::Own(Number(left * right));
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
	using Runtime::Number;

	int left = lhs->Execute(closure).TryAs<Number>()->GetValue();
	int right = rhs->Execute(closure).TryAs<Number>()->GetValue();
	return ObjectHolder::Own(Number(left / right));
}

ObjectHolder Compound::Execute(Closure& closure) {
	was_return = false;
	ObjectHolder holder;
	for (const auto& stmt : statements) {
		holder = stmt->Execute(closure);
		if (was_return) {
			return holder;
		}
	}
	return ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure& closure) {
	ObjectHolder holder = statement->Execute(closure);
	was_return = true;
	return holder;
}

ClassDefinition::ClassDefinition(ObjectHolder class_)
	: cls(move(class_)),
		class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
//	cout << class_name << endl;

	return cls;
}

FieldAssignment::FieldAssignment(
  VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
)
  : object(std::move(object))
  , field_name(std::move(field_name))
  , right_value(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
//	cout << "FieldAssignment: " << field_name << endl;
//	cout << "Closure: ";
//	for (const auto& [k, v] : closure) {
//		cout << k << ", ";
//	}
//	cout << endl;

//	closure[field_name] = right_value->Execute(closure);
//	return closure[field_name];

	ObjectHolder holder = object.Execute(closure);
	auto object_instance = holder.TryAs<Runtime::ClassInstance>();
	object_instance->Fields()[field_name] = right_value->Execute(closure);
	return object_instance->Fields()[field_name];
}

IfElse::IfElse(
  std::unique_ptr<Statement> condition,
  std::unique_ptr<Statement> if_body,
  std::unique_ptr<Statement> else_body
)
	: condition(move(condition)),
		if_body(move(if_body)),
		else_body(move(else_body))
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
	if (Runtime::IsTrue(condition->Execute(closure))) {
		return if_body->Execute(closure);
	} else {
		if (else_body) {
			return else_body->Execute(closure);
		} else {
			return ObjectHolder::None();
		}
	}
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
	using Runtime::Bool;

	bool left = lhs->Execute(closure).TryAs<Bool>()->GetValue();
	bool right = rhs->Execute(closure).TryAs<Bool>()->GetValue();
	return ObjectHolder::Own(Bool(left || right));
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
	using Runtime::Bool;

	bool left = lhs->Execute(closure).TryAs<Bool>()->GetValue();
	bool right = rhs->Execute(closure).TryAs<Bool>()->GetValue();
	return ObjectHolder::Own(Bool(left && right));
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
	using Runtime::Bool;

	bool arg = argument->Execute(closure).TryAs<Bool>()->GetValue();
	return ObjectHolder::Own(Bool(!arg));
}

Comparison::Comparison(
  Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
	: comparator(move(cmp)),
		left(move(lhs)),
		right(move(rhs))
{
}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
	bool result = comparator(left->Execute(closure), right->Execute(closure));
	return ObjectHolder::Own<Runtime::Bool>(move(result));
}

NewInstance::NewInstance(
  const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
)
  : class_(class_)
  , args(std::move(args))
{
}

NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {

}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
//	cout << "NewInstance: " << class_.GetName() << endl;

	vector<ObjectHolder> actual_args;
	actual_args.reserve(args.size());
	for (const auto& arg : args) {
		actual_args.push_back(arg->Execute(closure));
	}

	Runtime::ClassInstance object(class_);
	if (object.HasMethod("__init__", actual_args.size())) {
		object.Call("__init__", actual_args);
	}

//	cout << "NewInstance ptr: " << &object << endl;

	ObjectHolder holder = ObjectHolder::Own(move(object));
	holder.TryAs<Runtime::ClassInstance>()->Fields()["self"] = holder;
//	cout << "NewInstance ptr: " << holder.Get() << endl;

	return holder;
}


} /* namespace Ast */
