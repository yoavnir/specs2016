#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#define DEFAULT_PAD_CHAR ' '

class ProcessingState {
public:
	ProcessingState(char padChar=DEFAULT_PAD_CHAR) {m_pad = padChar;}
	ProcessingState(ProcessingState* pPS);
	ProcessingState(ProcessingState& ps);
	void    setPadChar(char padChar) {m_pad = padChar;}
	char    m_pad;
};

#endif
