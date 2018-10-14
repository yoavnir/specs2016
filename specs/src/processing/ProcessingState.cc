#include "ProcessingState.h"

ProcessingState::ProcessingState(ProcessingState& ps)
{
	m_pad = ps.m_pad;
}

ProcessingState::ProcessingState(ProcessingState* pPS)
{
	m_pad = pPS->m_pad;
}
