#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>
#include <iostream>
#include <stack>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
	if (HasMethod("__str__", 0)) {
		Call("__str__", {})->Print(os);
	} else {
		os << this;
	}
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
	const Method* mtd = cls.GetMethod(method);
	return mtd && mtd->formal_params.size() == argument_count;
}

const Closure& ClassInstance::Fields() const {
	return fields;
}

Closure& ClassInstance::Fields() {
	return fields;
}

ClassInstance::ClassInstance(const Class& cls) : cls(cls) {
//	fields["self"] = ObjectHolder::Share(*this);
}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
//	cout << "class: " << cls.GetName() << endl;
//	cout << "method: " << method << endl;
//	cout << "ptr: " << this << endl;
	const Method* mtd = cls.GetMethod(method);

	Closure closure;
	closure["self"] = ObjectHolder::Share(*this);

	for (size_t i = 0; i < mtd->formal_params.size(); ++i) {
		closure[mtd->formal_params[i]] = actual_args[i];
	}

//	if (method == "__init__") {
//		stack<const Method*> methods;
//		for (const Class* current = &cls; current; current = current->GetParent()) {
//			if (const Method* method = current->GetMethod("__init__")) {
//				methods.push(method);
//			}
//		}
//		while (!methods.empty()) {
//			methods.top()->body->Execute(fields);
//			methods.pop();
//		}
//		return ObjectHolder::None();
//	} else {
		return mtd->body->Execute(closure);
//	}
}

Class::Class(std::string name, std::vector<Method> methods_, const Class* parent)
	: name(move(name)), parent(parent)
{
	for (auto& method : methods_) {
		methods[method.name] = move(method);
	}
}

const Method* Class::GetMethod(const std::string& name) const {
	for (const Class* current = this; current; current = current->parent) {
		if (auto it = current->methods.find(name); it != current->methods.end()) {
			return &(it->second);
		}
	}
	return nullptr;
}

void Class::Print(ostream& os) {
	ClassInstance cls(*this);
	cls.Print(os);
}

const std::string& Class::GetName() const {
	return name;
}

void Bool::Print(std::ostream& os) {
	os << (GetValue() ? "True" : "False");
}

} /* namespace Runtime */
