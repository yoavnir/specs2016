#include <assert.h>
#include "specItems.h"

itemGroup::itemGroup()
{
	m_items.clear();
}

void itemGroup::addItem(Item *pItem)
{
	m_items.insert(m_items.end(), pItem);
}

void itemGroup::Debug()
{
	for (std::vector<PItem>::iterator it = m_items.begin(); it != m_items.end(); it++) {
		(*it)->Debug();
	}
}
