#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <sstream>

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
	if (lhs.TryAs<Number>()) {
		return lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue();
	}
	if (lhs.TryAs<String>()) {
		return lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue();
	}
	if (lhs.TryAs<Bool>()) {
		return lhs.TryAs<Bool>()->GetValue() == rhs.TryAs<Bool>()->GetValue();
	}
	return false;
}

bool Less(ObjectHolder lhs, ObjectHolder rhs) {
	if (lhs.TryAs<Number>()) {
		return lhs.TryAs<Number>()->GetValue() < rhs.TryAs<Number>()->GetValue();
	}
	if (lhs.TryAs<String>()) {
		return lhs.TryAs<String>()->GetValue() < rhs.TryAs<String>()->GetValue();
	}
	if (lhs.TryAs<Bool>()) {
		return lhs.TryAs<Bool>()->GetValue() < rhs.TryAs<Bool>()->GetValue();
	}
	return false;
}

} /* namespace Runtime */
