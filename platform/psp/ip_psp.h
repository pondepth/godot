/**************************************************************************/
/*  ip_psp.h                                                              */
/**************************************************************************/

#pragma once

#include "core/io/ip.h"

class IPPSP : public IP {
	GDCLASS(IPPSP, IP);

	static IP *_create_psp();
	void _resolve_hostname(List<IPAddress> &r_addresses, const String &p_hostname, Type p_type = TYPE_ANY) const override;

public:
	static void make_default();
	void get_local_interfaces(HashMap<String, Interface_Info> *r_interfaces) const override;
};

