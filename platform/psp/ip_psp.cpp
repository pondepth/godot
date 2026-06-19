/**************************************************************************/
/*  ip_psp.cpp                                                            */
/**************************************************************************/

#include "ip_psp.h"

IP *IPPSP::_create_psp() {
	return memnew(IPPSP);
}

void IPPSP::make_default() {
	_create = _create_psp;
}

void IPPSP::_resolve_hostname(List<IPAddress> &, const String &, Type) const {
}

void IPPSP::get_local_interfaces(HashMap<String, Interface_Info> *) const {
}
