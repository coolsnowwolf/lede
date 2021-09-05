-- Copyright 2011 Manuel Munz <freifunk at somakoma dot de>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.dns", package.seeall)

function rrdargs( graph, plugin, plugin_instance )

	local traffic = {
		title = "%H: DNS traffic", vlabel = "Bit/s",

		data = {
			sources = {
				dns_octets = { "queries", "responses" }
			},

			options = {
				dns_octets__responses = {
					total = true,
					color = "00ff00",
					title = "Responses"
				},

				dns_octets__queries = {
					total = true,
					color = "0000ff",
					title = "Queries"
				}
			}
		}
	}

	local opcode_query = {
		title = "%H: DNS Opcode Query", vlabel = "Queries/s",
		data = {
			instances = { dns_opcode = { "Query" } },
			options = {
				dns_opcode_Query_value = {
					total = true,
					color = "0000ff",
					title = "Queries/s"
				},
			}
		}
	}

	local qtype = {
		title = "%H: DNS QType", vlabel = "Queries/s",
		data = {
			sources = { dns_qtype = { "" } },
			options = {
				dns_qtype_AAAA_	= { title = "AAAA", noarea = true, total = true },
				dns_qtype_A_	= { title = "A", noarea = true, total = true },
				dns_qtype_A6_	= { title = "A6", noarea = true, total = true },
				dns_qtype_TXT_	= { title = "TXT", noarea = true, total = true },
				dns_qtype_MX_	= { title = "MX", noarea = true, total = true },
				dns_qtype_NS_	= { title = "NS", noarea = true, total = true },
				dns_qtype_ANY_	= { title = "ANY", noarea = true, total = true },
				dns_qtype_CNAME_= { title = "CNAME", noarea = true, total = true },
				dns_qtype_SOA_	= { title = "SOA", noarea = true, total = true },
				dns_qtype_SRV_	= { title = "SRV", noarea = true, total = true },
				dns_qtype_PTR_	= { title = "PTR", noarea = true, total = true },
				dns_qtype_RP_	= { title = "RP", noarea = true, total = true },
				dns_qtype_MAILB_= { title = "MAILB", noarea = true, total = true },
				dns_qtype_IXFR_	= { title = "IXFR", noarea = true, total = true },
				dns_qtype_HINFO_= { title = "HINFO", noarea = true, total = true },
			},
		}
	}
				
	return { traffic, opcode_query, qtype }
end
