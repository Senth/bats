#pragma once

#include <memory>
#include <map>

namespace bats {
	class ResourceGroup;
	typedef std::tr1::shared_ptr<ResourceGroup> ResourceGroupPtr;
	typedef std::tr1::shared_ptr<const ResourceGroup> ResourceGroupCstPtr;
	typedef std::map<int, ResourceGroupPtr>::iterator ResourceGroupIt;
	typedef std::map<int, ResourceGroupPtr>::const_iterator ResourceGroupCstIt;
}