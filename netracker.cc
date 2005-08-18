/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <stdio.h>
#include <time.h>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

#include "globalregistry.h"
#include "configfile.h"
#include "messagebus.h"
#include "packetchain.h"
#include "netracker.h"
#include "packet.h"
#include "gpsdclient.h"

// TCP server hooks
char *NETWORK_fields_text[] = {
    "bssid", "type", "ssid", "beaconinfo",
    "llcpackets", "datapackets", "cryptpackets",
    "weakpackets", "channel", "wep", "firsttime",
    "lasttime", "atype", "rangeip", "netmaskip",
	"gatewayip", "gpsfixed",
    "minlat", "minlon", "minalt", "minspd",
    "maxlat", "maxlon", "maxalt", "maxspd",
    "octets", "cloaked", "beaconrate", "maxrate",
    "manufkey", "manufscore",
    "quality", "signal", "noise",
    "bestquality", "bestsignal", "bestnoise",
    "bestlat", "bestlon", "bestalt",
    "agglat", "agglon", "aggalt", "aggpoints",
    "datasize",
    "turbocellnid", "turbocellmode", "turbocellsat",
    "carrierset", "maxseenrate", "encodingset",
    "decrypted", "dupeivpackets",
    NULL
};

char *REMOVE_fields_text[] = {
    "bssid",
    NULL
};

char *CLIENT_fields_text[] = {
    "bssid", "mac", "type", "firsttime", "lasttime",
    "manufkey", "manufscore",
    "datapackets", "cryptpackets", "weakpackets",
    "gpsfixed",
    "minlat", "minlon", "minalt", "minspd",
    "maxlat", "maxlon", "maxalt", "maxspd",
    "agglat", "agglon", "aggalt", "aggpoints",
    "maxrate",
    "quality", "signal", "noise",
    "bestquality", "bestsignal", "bestnoise",
    "bestlat", "bestlon", "bestalt",
    "atype", "ip", "datasize", "maxseenrate", "encodingset",
    "decrypted", "wep",
    NULL
};

// Network records.  data = NETWORK_data
int Protocol_NETWORK(PROTO_PARMS) {
	Netracker::tracked_network *net = (Netracker::tracked_network *) data;
	ostringstream osstr;

	// Alloc the cache quickly
	cache->Filled(field_vec->size());

    for (unsigned int x = 0; x < field_vec->size(); x++) {
        unsigned int fnum = (*field_vec)[x];
        if (fnum >= NETWORK_maxfield) {
            out_string = "Unknown field requested.";
            return -1;
		}

		osstr.str("");

		// Shortcut test the cache once and print/bail immediately
		if (cache->Filled(fnum)) {
			out_string += cache->GetCache(fnum) + " ";
			break;
		}

		// Fill in the cached element
		switch(fnum) {
			case NETWORK_bssid:
				cache->Cache(fnum, net->bssid.Mac2String());
				break;
			case NETWORK_type:
				osstr << net->type;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_ssid:
				if (net->ssid_cloaked)
					cache->Cache(fnum, "\001 \001");
				else
					cache->Cache(fnum, "\001" + net->ssid + "\001");
				break;
			case NETWORK_beaconinfo:
				if (net->beacon_info.length() == 0)
					cache->Cache(fnum, "\001 \001");
				else
					cache->Cache(fnum, "\001" + net->beacon_info + "\001");
				break;
			case NETWORK_llcpackets:
				osstr << net->llc_packets;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_datapackets:
				osstr << net->data_packets;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_cryptpackets:
				osstr << net->crypt_packets;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_weakpackets:
				osstr << net->fmsweak_packets;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_channel:
				osstr << net->channel;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_wep:
				osstr << net->cryptset;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_firsttime:
				osstr << (int) net->first_time;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_lasttime:
				osstr << (int) net->last_time;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_atype:
				osstr << (int) net->guess_ipdata.ip_type;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_rangeip:
				cache->Cache(fnum, inet_ntoa(net->guess_ipdata.ip_addr_block));
				break;
			case NETWORK_netmaskip:
				cache->Cache(fnum, inet_ntoa(net->guess_ipdata.ip_netmask));
				break;
			case NETWORK_gatewayip:
				cache->Cache(fnum, inet_ntoa(net->guess_ipdata.ip_gateway));
				break;
			case NETWORK_gpsfixed:
				osstr << net->gpsdata.gps_valid;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_minlat:
				osstr << net->gpsdata.min_lat;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_minlon:
				osstr << net->gpsdata.min_lon;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_minalt:
				osstr << net->gpsdata.min_alt;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_minspd:
				osstr << net->gpsdata.min_spd;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxlat:
				osstr << net->gpsdata.max_lat;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxlon:
				osstr << net->gpsdata.max_lon;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxalt:
				osstr << net->gpsdata.max_alt;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxspd:
				osstr << net->gpsdata.max_spd;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_octets:
				// Deprecated
				cache->Cache(fnum, "0");
				break;
			case NETWORK_cloaked:
				if (net->ssid_cloaked)
					cache->Cache(fnum, "1");
				else
					cache->Cache(fnum, "0");
				break;
			case NETWORK_beaconrate:
				osstr << net->beaconrate;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxrate:
				osstr << net->maxrate;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_manufkey:
			case NETWORK_manufscore:
				// Deprecated/broken
				// FIXME manfkey
				cache->Cache(fnum, "0");
				break;
			case NETWORK_quality:
			case NETWORK_bestquality:
				// Deprecated
				cache->Cache(fnum, "0");
				break;
			case NETWORK_signal:
				osstr << net->snrdata.last_signal;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_noise:
				osstr << net->snrdata.last_noise;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_bestsignal:
				osstr << net->snrdata.max_signal;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_bestnoise:
				osstr << net->snrdata.max_noise;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_bestlat:
				osstr << net->snrdata.peak_lat;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_bestlon:
				osstr << net->snrdata.peak_lon;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_bestalt:
				osstr << net->snrdata.peak_alt;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_agglat:
				osstr << net->gpsdata.aggregate_lat;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_agglon:
				osstr << net->gpsdata.aggregate_lon;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_aggalt:
				osstr << net->gpsdata.aggregate_alt;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_aggpoints:
				osstr << net->gpsdata.aggregate_points;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_datasize:
				osstr << net->datasize;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_tcnid:
				// FIXME turbocell
				cache->Cache(fnum, "\001 \001");
				break;
			case NETWORK_tcmode:
			case NETWORK_tsat:
				// FIXME turbocell
				cache->Cache(fnum, "0");
				break;
			case NETWORK_carrierset:
				osstr << net->snrdata.carrierset;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_maxseenrate:
				osstr << net->snrdata.maxseenrate;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_encodingset:
				osstr << net->snrdata.encodingset;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_decrypted:
				osstr << net->decrypted;
				cache->Cache(fnum, osstr.str());
				break;
			case NETWORK_dupeiv:
				osstr << net->dupeiv_packets;
				cache->Cache(fnum, osstr.str());
				break;
		}

		// print the newly filled in cache
		out_string += cache->GetCache(fnum) + " ";
    }

    return 1;
}

void Protocol_Client2Data(const Netracker::tracked_network *net, 
						  const Netracker::tracked_client *cli, CLIENT_data *data) {
#if 0
    char tmpstr[128];

    // Reserve fields
    data->cdvec.reserve(50);

    data->cdvec.push_back(net->bssid.Mac2String());

    data->cdvec.push_back(cli->mac.Mac2String());

    snprintf(tmpstr, 128, "%d", (int) cli->type);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", (int) cli->first_time);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", (int) cli->last_time);
    data->cdvec.push_back(tmpstr);

    // Deprecated
    // data->cdvec.push_back(cli->manuf_key.Mac2String());
    data->cdvec.push_back("00:00:00:00:00:00");

    // deprecated
    /*
    snprintf(tmpstr, 128, "%d", cli->manuf_score);
    data->cdvec.push_back(tmpstr);
    */
    data->cdvec.push_back("0");

    snprintf(tmpstr, 128, "%d", cli->data_packets);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->crypt_packets);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->interesting_packets);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->gps_fixed);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->min_lat);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->min_lon);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->min_alt);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->min_spd);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->max_lat);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->max_lon);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->max_alt);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->max_spd);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->aggregate_lat);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->aggregate_lon);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->aggregate_alt);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%ld", cli->aggregate_points);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%2.1f", cli->maxrate);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->signal);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->noise);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->best_signal);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->best_noise);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->best_lat);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->best_lon);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%f", cli->best_alt);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", (int) cli->ipdata.atype);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%hd.%hd.%hd.%hd",
             cli->ipdata.ip[0], cli->ipdata.ip[1],
             cli->ipdata.ip[2], cli->ipdata.ip[3]);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%ld", cli->datasize);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->maxseenrate);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->encoding_set);
    data->cdvec.push_back(tmpstr);

    snprintf(tmpstr, 128, "%d", cli->decrypted);
    data->cdvec.push_back(tmpstr);

	// Crypt set goes here
	snprintf(tmpstr, 128, "%d", 0);
	data->cdvec.push_back(tmpstr);
#endif

}

// client records.  data = CLIENT_data
int Protocol_CLIENT(PROTO_PARMS) {
    CLIENT_data *cdata = (CLIENT_data *) data;

    for (unsigned int x = 0; x < field_vec->size(); x++) {
        unsigned int fnum = (*field_vec)[x];
        if (fnum >= cdata->cdvec.size()) {
            out_string = "Unknown field requested.";
            return -1;
        } else {
            out_string += cdata->cdvec[fnum] + " ";
        }
    }

    return 1;
}

int Protocol_REMOVE(PROTO_PARMS) {
    string *str = (string *) data;
    out_string += *str;
    return 1;
}

void Protocol_NETWORK_enable(PROTO_ENABLE_PARMS) {
	printf("debug - network enable\n");
	// Bad touch, bad touch!
	for (Netracker::track_iter x = globalreg->netracker->tracked_map.begin(); 
		 x != globalreg->netracker->tracked_map.end(); ++x) {
        if (x->second->type == network_remove) 
            continue;

		// Send with a local cache that just gets thrown away, its only to 1
		// client so we can't efficiently cache
		kis_protocol_cache cache;
		globalreg->kisnetserver->SendToClient(in_fd, _NPM(PROTO_REF_NETWORK),
											  (void *) x->second, &cache);
	}
}

void Protocol_CLIENT_enable(PROTO_ENABLE_PARMS) {
#if 0
    vector<wireless_network *> tracked;
    tracked = globalreg->packetracker->FetchNetworks();

    for (unsigned int x = 0; x < tracked.size(); x++) {
        for (map<mac_addr, wireless_client *>::const_iterator y = tracked[x]->client_map.begin();
             y != tracked[x]->client_map.end(); ++y) {
            CLIENT_data cdata;
            Protocol_Client2Data(tracked[x], y->second, &cdata);
            globalreg->kisnetserver->SendToClient(in_fd, globalreg->cli_prot_ref, (void *) &cdata);
        }
    }
#endif
}

// These are both just dropthroughs into the class itself
int kis_80211_netracker_hook(CHAINCALL_PARMS) {
	Netracker *auxptr = (Netracker *) auxdata;
	return auxptr->netracker_chain_handler(in_pack);
}

int kis_80211_datatracker_hook(CHAINCALL_PARMS) {
	Netracker *auxptr = (Netracker *) auxdata;
	return 0;

	// return auxptr->datatracker_chain_handler(in_pack);
}

Netracker::Netracker() {
	fprintf(stderr, "FATAL OOPS: Netracker() called with no global registry\n");
}

Netracker::Netracker(GlobalRegistry *in_globalreg) {
	globalreg = in_globalreg;

	// Sanity
	if (globalreg->packetchain == NULL) {
		fprintf(stderr, "Netracker() Sanity failed, globalreg->packetchain is NULL, "
				"can't continue\n");
		exit(1);
	}

	if (globalreg->kisnetserver == NULL) {
		fprintf(stderr, "Netracker() sanity failed, globalreg->netserver is NULL\n");
		exit(1);
	}
	
	if (globalreg->kismet_config == NULL) {
		fprintf(stderr, "Netracker() sanity failed, globalreg->config is NULL\n");
		exit(1);
	}

	// Register packet components to tie into our tracker
	_PCM(PACK_COMP_TRACKERNET) =
		globalreg->packetchain->RegisterPacketComponent("netracker_network");
	_PCM(PACK_COMP_TRACKERCLIENT) =
		globalreg->packetchain->RegisterPacketComponent("netracker_client");

	// Register the packet hooks with the chain
	globalreg->packetchain->RegisterHandler(&kis_80211_netracker_hook, this,
											CHAINPOS_CLASSIFIER, -100);
	globalreg->packetchain->RegisterHandler(&kis_80211_datatracker_hook, this,
											CHAINPOS_CLASSIFIER, -99);

	if (globalreg->kismet_config->FetchOpt("track_probenets") == "true") {
		_MSG("Probe network tracking enabled by config file", MSGFLAG_INFO);
		track_probenets = 1;
	} else {
		_MSG("Probe network tracking disabled by config file", MSGFLAG_INFO);
		track_probenets = 0;
	}
	
	string config_path;
	if ((config_path = globalreg->kismet_config->FetchOpt("configdir")) != "") {
		if ((ssid_cache_path = globalreg->kismet_config->FetchOpt("ssidmap")) != "") {
			ssid_cache_path = 
				globalreg->kismet_config->ExpandLogPath(config_path + ssid_cache_path,
														"", "", 0, 1);
			ReadSSIDCache();
		} else {
			ssid_cache_track = 0;
		}

		if ((ip_cache_path = globalreg->kismet_config->FetchOpt("ipmap")) != "") {
			ip_cache_path = 
				globalreg->kismet_config->ExpandLogPath(config_path + ip_cache_path,
														"", "", 0, 1);
			ReadIPCache();
		} else {
			ssid_cache_track = 0;
		}

	} else {
		ssid_cache_track = 0;
		ssid_cache_track = 0;
	}

	// Register network protocols with the tcp server
	_NPM(PROTO_REF_NETWORK) =
		globalreg->kisnetserver->RegisterProtocol("NETWORK", 0, 1, 
												  NETWORK_fields_text, 
												  &Protocol_NETWORK, 
												  &Protocol_NETWORK_enable);
	_NPM(PROTO_REF_CLIENT) =
		globalreg->kisnetserver->RegisterProtocol("CLIENT", 0, 1,
												  CLIENT_fields_text, 
												  &Protocol_CLIENT, 
												  &Protocol_CLIENT_enable);
	_NPM(PROTO_REF_REMOVE) =
		globalreg->kisnetserver->RegisterProtocol("REMOVE", 0, 1,
												  REMOVE_fields_text, 
												  &Protocol_REMOVE, NULL);

	// TODO:
	// Register timer events
	
}

int Netracker::netracker_chain_handler(kis_packet *in_pack) {
	tracked_network *net = NULL;
	char status[STATUS_MAX];

	// Fetch the info from the packet chain data
	kis_ieee80211_packinfo *packinfo = (kis_ieee80211_packinfo *) 
		in_pack->fetch(_PCM(PACK_COMP_80211));
	kis_gps_packinfo *gpsinfo = (kis_gps_packinfo *) 
		in_pack->fetch(_PCM(PACK_COMP_GPS));
	kis_layer1_packinfo *l1info = (kis_layer1_packinfo *) 
		in_pack->fetch(_PCM(PACK_COMP_RADIODATA));

	// No 802.11 info, we don't handle it.
	if (packinfo == NULL) {
		return 0;
	}

	// Not an 802.11 frame type we known how to track, we'll just skip
	// it, too
	if (packinfo->corrupt || packinfo->type == packet_noise ||
		packinfo->type == packet_unknown || 
		packinfo->subtype == packet_sub_unknown) {
		return 0;
	}

	// Look to see if we already track this bssid and grab it if we do
	track_iter triter = tracked_map.find(packinfo->bssid_mac);
	if (triter != tracked_map.end())
		net = triter->second;

	// Try to map probe reqs into the network they really belong in, if we
	// track probes, and we don't already have a network for them
	if (track_probenets && 
		net == NULL &&
		packinfo->type == packet_management &&
		packinfo->subtype == packet_sub_probe_req) {
		if (probe_assoc_map.find(packinfo->bssid_mac) != probe_assoc_map.end()) {
			net = probe_assoc_map[packinfo->bssid_mac];
		}
	} 

	// TODO: Adhoc and inter-ds matching needs to go here once its redone

	// Spawn a new network record
	if (net == NULL) {
		// Constructor will make our network record clear
		net = new Netracker::tracked_network;

		// Cached IP data
		if (bssid_ip_map.find(packinfo->bssid_mac) != bssid_ip_map.end()) {
			net->guess_ipdata = bssid_ip_map[packinfo->bssid_mac];
		}

		// Copy management network info in
		if (packinfo->type == packet_management &&
			packinfo->subtype == packet_sub_beacon) {

			// Find cached SSID if we don't have one
			if (packinfo->ssid_len == 0 || packinfo->ssid_blank) {
				net->ssid_cloaked = 1;
				if (bssid_cloak_map.find(packinfo->bssid_mac) != 
					bssid_cloak_map.end()) {
					net->ssid = bssid_cloak_map[packinfo->bssid_mac];
					net->ssid_uncloaked = 1;
				}
			} else {
				net->ssid = string(packinfo->ssid);
				net->ssid_cloaked = 0;
			}
		}

		net->bssid = packinfo->bssid_mac;

		if (packinfo->type == packet_management && 
			packinfo->subtype == packet_sub_probe_req) {
			net->type = network_probe;
		} else if (packinfo->distrib == distrib_adhoc) {
			net->type = network_adhoc;
		} else {
			net->type = network_ap;
		}

		net->first_time = time(0);
		net->bss_timestamp = packinfo->timestamp;

		// Learn it
		tracked_map[net->bssid] = net;

		// Everything else needs to change with new frames so we fill it in
		// outside of the new network code, obviously
		snprintf(status, STATUS_MAX, "Detected new network '%s' BSSID %s",
				 (net->ssid_cloaked && !net->ssid_uncloaked) ? 
				 	"<no ssid>" : net->ssid.c_str(), 
					net->bssid.Mac2String().c_str());

		_MSG(status, MSGFLAG_INFO);
	}

	// Link it to the packet for future chain elements
	kis_netracker_netinfo *netpackinfo = new kis_netracker_netinfo;
	netpackinfo->netref = net;
	in_pack->insert(globalreg->packetcomp_map[PACK_COMP_TRACKERNET], netpackinfo);

	// Update the time
	net->last_time = time(0);

	// Dirty the network
	net->dirty = 1;

	// Extract info from the GPS component, if we have one
	if (gpsinfo != NULL) {
		net->gpsdata.gps_valid = 1;

		if (gpsinfo->lat < net->gpsdata.min_lat)
			net->gpsdata.min_lat = gpsinfo->lat;
		if (gpsinfo->lon < net->gpsdata.min_lon)
			net->gpsdata.min_lon = gpsinfo->lon;
		if (gpsinfo->alt < net->gpsdata.min_alt)
			net->gpsdata.min_alt = gpsinfo->alt;
		if (gpsinfo->spd < net->gpsdata.min_spd)
			net->gpsdata.min_spd = gpsinfo->spd;

		if (gpsinfo->lat > net->gpsdata.max_lat)
			net->gpsdata.max_lat = gpsinfo->lat;
		if (gpsinfo->lon > net->gpsdata.max_lon)
			net->gpsdata.max_lon = gpsinfo->lon;
		if (gpsinfo->alt > net->gpsdata.max_alt)
			net->gpsdata.max_alt = gpsinfo->alt;
		if (gpsinfo->spd > net->gpsdata.max_spd)
			net->gpsdata.max_spd = gpsinfo->spd;

		net->gpsdata.aggregate_lat += gpsinfo->lat;
		net->gpsdata.aggregate_lon += gpsinfo->lon;
		net->gpsdata.aggregate_alt += gpsinfo->alt;
		net->gpsdata.aggregate_points++;
	}

	// L1 signal info, if our capture source was able to inject any into
	// the packet.
	if (l1info != NULL) {
		net->snrdata.last_signal = l1info->signal;
		net->snrdata.last_noise = l1info->noise;

		if (l1info->noise > net->snrdata.max_noise) {
			net->snrdata.max_noise = l1info->noise;
		}

		if (l1info->signal > net->snrdata.max_signal) {
			net->snrdata.max_signal = l1info->signal;

			if (gpsinfo != NULL) {
				net->snrdata.peak_lat = gpsinfo->lat;
				net->snrdata.peak_lon = gpsinfo->lon;
				net->snrdata.peak_alt = gpsinfo->alt;
			}
		}

		if (l1info->datarate < net->snrdata.maxseenrate)
			net->snrdata.maxseenrate = l1info->datarate;

		// Push in the bits for the carrier and encoding
		net->carrier_set |= (1 << (int) l1info->carrier);
		net->encoding_set |= (1 << (int) l1info->encoding);
	}

	// Extract info from beacon frames, they're the only ones we trust to
	// give us good info...
	if (packinfo->type == packet_management && 
		packinfo->subtype == packet_sub_beacon) {
		net->beacon_info = string(packinfo->beacon_info);

		if (packinfo->ssid_len != 0 && packinfo->ssid_blank != 0) {
			net->ssid = string(packinfo->ssid);
		}

		if (net->maxrate < packinfo->maxrate)
			net->maxrate = packinfo->maxrate;

		if (packinfo->wep)
			net->cryptset |= crypt_wep;

		net->channel = packinfo->channel;

		net->beaconrate = packinfo->beacon_interval;
	}

	// Catch probe responses and decloak if they're nonblank
	if (net->ssid_cloaked != 0 && net->ssid_uncloaked == 0 &&
		packinfo->type == packet_management &&
		packinfo->subtype == packet_sub_probe_resp &&
		packinfo->ssid_len != 0 && packinfo->ssid_blank != 0) {

		net->ssid_uncloaked = 1;
		net->ssid = packinfo->ssid;
		// Update the cloak map
		bssid_cloak_map[net->bssid] = packinfo->ssid;

		snprintf(status, STATUS_MAX, "Discovered SSID \"%s\" for cloaked network %s",
				 net->ssid.c_str(), net->bssid.Mac2String().c_str());
		globalreg->messagebus->InjectMessage(status, MSGFLAG_INFO);
	}

	if (packinfo->type == packet_management ||
		packinfo->type == packet_phy) {
		net->llc_packets++;
	} else if (packinfo->type == packet_data) {
		net->data_packets++;

		if (packinfo->encrypted)
			net->crypt_packets++;
	}

	// TODO:  
	//  FMSWEAK packets
	//  Manuf matching

	return 1;
}

int Netracker::ReadSSIDCache() {
	FILE *ssidf;
	char errstr[1024];
	int ver;

	if ((ssidf = fopen(ssid_cache_path.c_str(), "r")) == NULL) {
		snprintf(errstr, 1024, "Netracker failed to read SSID cache file '%s':  %s",
				 ssid_cache_path.c_str(), strerror(errno));
		globalreg->messagebus->InjectMessage(errstr, MSGFLAG_ERROR);
		return -1;
	}

	// Hijack the error string as a convenient buffer
	fgets(errstr, 1024, ssidf);
	if (sscanf(errstr, "SSIDCACHE_VERSION: %d\n", &ver) != 1) {
		globalreg->messagebus->InjectMessage("Netracker failed to read SSID cache "
											 "file version, cache file will be "
											 "replaced", MSGFLAG_ERROR);
		fclose(ssidf);
		return 0;
	}

	if (ver != NETRACKER_SSIDCACHE_VERSION) {
		snprintf(errstr, 1024, "Netracker got different SSID Cache version, cache "
				 "file will be replaced.  (Got %d expected %d)", ver,
				 NETRACKER_SSIDCACHE_VERSION);
		fclose(ssidf);
		return 0;
	}

	do {
		char macstr[18];
		char ssid[64];
		mac_addr mac;

		// Keep hijacking the error buffer
		fgets(errstr, 1024, ssidf);

		if (sscanf(errstr, "%18s \001%64[^\001]\001\n", macstr, ssid) != 2) {
			globalreg->messagebus->InjectMessage("Netracker got invalid line in "
												 "SSID cache file, skipping",
												 MSGFLAG_INFO);
			continue;
		}

		mac = macstr;
		if (mac.error) {
			globalreg->messagebus->InjectMessage("Netracker got invalid MAC address "
												 "in SSID cache file, skipping",
												 MSGFLAG_INFO);
			continue;
		}

		bssid_cloak_map[mac] = string(ssid);
	} while (!feof(ssidf));

	fclose(ssidf);

	return 1;
}

int Netracker::WriteSSIDCache() {
	FILE *ssidf;
	char errstr[1024];

	if ((ssidf = fopen(ssid_cache_path.c_str(), "w")) == NULL) {
		snprintf(errstr, 1024, "Netracker failed to open SSID cache file '%s':  %s",
				 ssid_cache_path.c_str(), strerror(errno));
		globalreg->messagebus->InjectMessage(errstr, MSGFLAG_ERROR);
	}

	fprintf(ssidf, "SSIDCACHE_VERSION: %d\n", NETRACKER_SSIDCACHE_VERSION);

	// Write out everything in the cache map (this must be updated as new networks
	// are found/uncloaked)
	for (ssidcache_iter x = bssid_cloak_map.begin(); 
		 x != bssid_cloak_map.end(); ++x) {
		fprintf(ssidf, "%s \001%64s\001\n", x->first.Mac2String().c_str(),
				x->second.c_str());
	}

	fclose(ssidf);

	return 1;
}

int Netracker::ReadIPCache() {
	FILE *ipf;
	char errstr[1024];
	int ver;

	if ((ipf = fopen(ip_cache_path.c_str(), "r")) == NULL) {
		snprintf(errstr, 1024, "Netracker failed to read IP cache file '%s':  %s",
				 ip_cache_path.c_str(), strerror(errno));
		globalreg->messagebus->InjectMessage(errstr, MSGFLAG_ERROR);
		return -1;
	}

	// Hijack the error string as a convenient buffer
	fgets(errstr, 1024, ipf);
	if (sscanf(errstr, "IPCACHE_VERSION: %d\n", &ver) != 1) {
		globalreg->messagebus->InjectMessage("Netracker failed to read IP cache "
											 "file version, cache file will be "
											 "replaced", MSGFLAG_ERROR);
		fclose(ipf);
		return 0;
	}

	if (ver != NETRACKER_IPCACHE_VERSION) {
		snprintf(errstr, 1024, "Netracker got different IP Cache version, cache "
				 "file will be replaced.  (Got %d expected %d)", ver,
				 NETRACKER_IPCACHE_VERSION);
		fclose(ipf);
		return 0;
	}

	do {
		char macstr[18];
		ip_data ipd;
		mac_addr mac;
		int ipaddr, netmask, gateway;

		// Keep hijacking the error buffer
		fgets(errstr, 1024, ipf);

		if (sscanf(errstr, "%18s %d %d %d\n", macstr, &ipaddr, 
				   &netmask, &gateway) != 4) {
			globalreg->messagebus->InjectMessage("Netracker got invalid line in "
												 "IP cache file, skipping",
												 MSGFLAG_INFO);
			continue;
		}

		mac = macstr;
		if (mac.error) {
			globalreg->messagebus->InjectMessage("Netracker got invalid MAC address "
												 "in IP cache file, skipping",
												 MSGFLAG_INFO);
			continue;
		}

		ipd.ip_addr_block.s_addr = ipaddr;
		ipd.ip_netmask.s_addr = netmask;
		ipd.ip_gateway.s_addr = gateway;

		bssid_ip_map[mac] = ipd;
	} while (!feof(ipf));

	fclose(ipf);

	return 1;
}

int Netracker::WriteIPCache() {
	FILE *ipf;
	char errstr[1024];

	if ((ipf = fopen(ip_cache_path.c_str(), "w")) == NULL) {
		snprintf(errstr, 1024, "Netracker failed to open IP cache file '%s':  %s",
				 ip_cache_path.c_str(), strerror(errno));
		globalreg->messagebus->InjectMessage(errstr, MSGFLAG_ERROR);
	}

	fprintf(ipf, "IPCACHE_VERSION: %d\n", NETRACKER_IPCACHE_VERSION);

	// If we're cached and don't exist in the real map, write it out
	// If we're in the real map, write that out instead
	for (ipcache_iter x = bssid_ip_map.begin(); x != bssid_ip_map.end(); ++x) {
		track_iter triter;
		ip_data ipd;
		
		if ((triter = tracked_map.find(x->first)) != tracked_map.end())
			ipd = triter->second->guess_ipdata;
		else
			ipd = x->second;

		fprintf(ipf, "%s %d %d %d\n", x->first.Mac2String().c_str(),
				x->second.ip_addr_block, x->second.ip_netmask,
				x->second.ip_gateway);
	}

	fclose(ipf);

	return 1;
}


