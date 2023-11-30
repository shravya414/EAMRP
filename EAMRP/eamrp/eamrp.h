


#ifndef __eamrp_h__
#define __eamrp_h__

//#include <agent.h>
//#include <packet.h>
//#include <sys/types.h>
//#include <cmu/list.h>
//#include <scheduler.h>

#include <cmu-trace.h>
#include <priqueue.h>
#include <eamrp/eamrp_rtable.h>
#include <eamrp/eamrp_rqueue.h>
// AODV ns-2.31 code
#include <classifier/classifier-port.h>

// EAMRP code
#define EAMRP_PACKET_SALVAGING
#define EAMRP_MAX_SALVAGE_COUNT  10
#define EAMRP_EXPANDING_RING_SEARCH

// EAMRP code
//#define EAMRP_LINK_DISJOINT_PATHS
#define EAMRP_NODE_DISJOINT_PATHS

/*
  Allows local repair of routes 
*/
// EAMRP code - could it be uncomment?
#define EAMRP_LOCAL_REPAIR

/*
  Allows AODV to use link-layer (802.11) feedback in determining when
  links are up/down.
*/
//#define EAMRP_LINK_LAYER_DETECTION

/*
  Causes AODV to apply a "smoothing" function to the link layer feedback
  that is generated by 802.11.  In essence, it requires that RT_MAX_ERROR
  errors occurs within a window of RT_MAX_ERROR_TIME before the link
  is considered bad.
*/
#define EAMRP_USE_LL_METRIC

/*
  Only applies if AODV_USE_LL_METRIC is defined.
  Causes AODV to apply omniscient knowledge to the feedback received
  from 802.11.  This may be flawed, because it does not account for
  congestion.
*/
//#define EAMRP_USE_GOD_FEEDBACK


class EAMRP;

#define MY_ROUTE_TIMEOUT        10                       // 100 seconds
#define ACTIVE_ROUTE_TIMEOUT    10          // 50 seconds
#define REV_ROUTE_LIFE          6            // 5  seconds
// AODV ns-2.31 code
#define BCAST_ID_SAVE           6            // 3 seconds


// No. of times to do network-wide search before timing out for 
// MAX_RREQ_TIMEOUT sec. 
#define RREQ_RETRIES            3  
// timeout after doing network-wide search RREQ_RETRIES times
#define MAX_RREQ_TIMEOUT   1.0 //sec

/* Various constants used for the expanding ring search */
// EAMRP code
#ifdef EAMRP_EXPANDING_RING_SEARCH
#define TTL_START     		5		// 5
#define TTL_INCREMENT 		2 		// 2
#else // NO EXPANDING RING SEARCH
#define TTL_START     		30
#define TTL_INCREMENT 		30
#endif // NO EXPANDING RING SEARCH
#define TTL_THRESHOLD 7

// Should be set by the user using best guess (conservative) 
#define NETWORK_DIAMETER        30             // 30 hops

// Must be larger than the time difference between a node propagates a route 
// request and gets the route reply back.

//#define RREP_WAIT_TIME     (3 * NODE_TRAVERSAL_TIME * NETWORK_DIAMETER) // ms
//#define RREP_WAIT_TIME     (2 * REV_ROUTE_LIFE)  // seconds
#define RREP_WAIT_TIME         1.0  // sec

#define ID_NOT_FOUND    0x00
#define ID_FOUND        0x01
//#define INFINITY        0xff

// The followings are used for the forward() function. Controls pacing.
#define EAMRP_DELAY 1.0           // random delay
#define NO_EAMRP_DELAY -1.0       // no delay 

// think it should be 30 ms
#define ARP_DELAY 0.01      // fixed delay to keep arp happy


#define HELLO_INTERVAL          1               // 1000 ms
#define ALLOWED_HELLO_LOSS      3               // packets
#define BAD_LINK_LIFETIME       3               // 3000 ms
#define MaxHelloInterval        (1.25 * HELLO_INTERVAL)
#define MinHelloInterval        (0.75 * HELLO_INTERVAL)

// EAMRP code - Could it be removev?
// This should be somewhat related to arp timeout
#define NODE_TRAVERSAL_TIME     0.03             // 30 ms
#define LOCAL_REPAIR_WAIT_TIME  0.15 //sec



/*
  Timers (Broadcast ID, Hello, Neighbor Cache, Route Cache)
*/
class EAMRPBroadcastTimer : public Handler {
public:
        EAMRPBroadcastTimer(EAMRP* a) : agent(a) {}
        void   handle(Event*);
private:
        EAMRP    *agent;
   Event intr;
};

class EAMRPHelloTimer : public Handler {
public:
        EAMRPHelloTimer(EAMRP* a) : agent(a) {}
        void   handle(Event*);
private:
        EAMRP    *agent;
   Event intr;
};

class EAMRPNeighborTimer : public Handler {
public:
        EAMRPNeighborTimer(EAMRP* a) : agent(a) {}
        void   handle(Event*);
private:
        EAMRP    *agent;
   Event intr;
};

class EAMRPRouteCacheTimer : public Handler {
public:
        EAMRPRouteCacheTimer(EAMRP* a) : agent(a) {}
        void   handle(Event*);
private:
        EAMRP    *agent;
   Event intr;
};

class EAMRPLocalRepairTimer : public Handler {
public:
        EAMRPLocalRepairTimer(EAMRP* a) : agent(a) {}
        void   handle(Event*);
private:
        EAMRP    *agent;
   Event intr;
};

// EAMRP code
/*
  Route List
*/
class EAMRP_Route {
        friend class EAMRPBroadcastID;
 public:
        EAMRP_Route(nsaddr_t nexthop, nsaddr_t lasthop=0) {
				nh_addr = nexthop;
				lh_addr = lasthop;
		  }
	protected:
	LIST_ENTRY(EAMRP_Route) route_link;
        nsaddr_t        nh_addr;
        nsaddr_t        lh_addr;
};

LIST_HEAD(eamrp_routes, EAMRP_Route);


/*
  Broadcast ID Cache
*/
class EAMRPBroadcastID {
        friend class EAMRP;
 public:
        EAMRPBroadcastID(nsaddr_t i, u_int32_t b) { 
				src = i; 
				id = b;  
				// EAMRP code
				count=0;  
				LIST_INIT(&reverse_path_list);
				LIST_INIT(&forward_path_list);
        }
protected:
        LIST_ENTRY(EAMRPBroadcastID) link;
        nsaddr_t        src;
        u_int32_t       id;
        double          expire;         // now + BCAST_ID_SAVE s
        
			// EAMRP code
        int					count;
        eamrp_routes     reverse_path_list;     // List of reverse paths used for forwarding replies
        eamrp_routes     forward_path_list;     // List of forward paths advertised already

        inline EAMRP_Route* reverse_path_insert(nsaddr_t nexthop, nsaddr_t lasthop=0) {
			  EAMRP_Route* route = new EAMRP_Route(nexthop, lasthop);
			  assert(route);
			  LIST_INSERT_HEAD(&reverse_path_list, route, route_link);
			  return route;
        }

        inline EAMRP_Route* reverse_path_lookup(nsaddr_t nexthop, nsaddr_t lasthop=0) {
			  EAMRP_Route *route = reverse_path_list.lh_first;

			  // Search the list for a match of id
			  for( ; route; route = route->route_link.le_next) {
					if ( (route->nh_addr == nexthop) && (route->lh_addr == lasthop) )
						return route;     
					}
				return NULL;
				}
		  
			inline EAMRP_Route* forward_path_insert(nsaddr_t nexthop, nsaddr_t lasthop=0) {
				EAMRP_Route* route = new EAMRP_Route(nexthop, lasthop);
				assert(route);
				LIST_INSERT_HEAD(&forward_path_list, route, route_link);
				return route;
			}

			inline EAMRP_Route* forward_path_lookup(nsaddr_t nexthop, nsaddr_t lasthop=0) {
				EAMRP_Route *route = forward_path_list.lh_first;

				// Search the list for a match of id
				for( ; route; route = route->route_link.le_next) {
					if ( (route->nh_addr == nexthop) &&	(route->lh_addr == lasthop) ) 
						return route;     
				}
				return NULL;
			}
};

LIST_HEAD(eamrp_bcache, EAMRPBroadcastID);


/*
  The Routing Agent
*/
class EAMRP: public Agent {

  /*
   * make some friends first 
   */

        friend class eamrp_rt_entry;
        friend class EAMRPBroadcastTimer;
        friend class EAMRPHelloTimer;
        friend class EAMRPNeighborTimer;
        friend class EAMRPRouteCacheTimer;
        friend class EAMRPLocalRepairTimer;

 public:
        EAMRP(nsaddr_t id);

        void      recv(Packet *p, Handler *);

 protected:
        int             command(int, const char *const *);
        int             initialized() { return 1 && target_; }

        /*
         * Route Table Management
         */
        void            rt_resolve(Packet *p);
        void            rt_down(eamrp_rt_entry *rt);
        void            local_rt_repair(eamrp_rt_entry *rt, Packet *p);
 public:
        void            rt_ll_failed(Packet *p);
			// EAMRP code
			// void            rt_update(aodv_rt_entry *rt, u_int32_t seqnum, u_int16_t metric, nsaddr_t nexthop, double expire_time);
			// void            handle_link_failure(nsaddr_t id);
        void            handle_link_failure(nsaddr_t id);
 protected:
        void            rt_purge(void);

        void            enque(eamrp_rt_entry *rt, Packet *p);
        Packet*         deque(eamrp_rt_entry *rt);

        /*
         * Neighbor Management
         */
        void            nb_insert(nsaddr_t id);
        EAMRP_Neighbor*       nb_lookup(nsaddr_t id);
        void            nb_delete(nsaddr_t id);
        void            nb_purge(void);

        /*
         * Broadcast ID Management
         */

			// AODV ns-2.31 code
			void            id_insert(nsaddr_t id, u_int32_t bid);
			bool	        id_lookup(nsaddr_t id, u_int32_t bid);
         EAMRPBroadcastID*    id_get(nsaddr_t id, u_int32_t bid);
        void            id_purge(void);

        /*
         * Packet TX Routines
         */
        void            forward(eamrp_rt_entry *rt, Packet *p, double delay);
			// EAMRP code - should it be removed?
        void            forwardReply(eamrp_rt_entry *rt, Packet *p, double delay);
        void            sendHello(void);
        void            sendRequest(nsaddr_t dst);

			// EAMRP code
			// void            sendReply(nsaddr_t ipdst, u_int32_t hop_count, nsaddr_t rpdst, u_int32_t rpseq, u_int32_t lifetime, double timestamp);
        void            sendReply(nsaddr_t ipdst, u_int32_t hop_count,
                                  nsaddr_t rpdst, u_int32_t rpseq,
                                  double lifetime, double timestamp,
              nsaddr_t nexthop, u_int32_t bcast_id, nsaddr_t rp_first_hop);
        void            sendError(Packet *p, bool jitter = true);
                                          
        /*
         * Packet RX Routines
         */
        void            recvEAMRP(Packet *p);
        void            recvHello(Packet *p);
        void            recvRequest(Packet *p);
        void            recvReply(Packet *p);
        void            recvError(Packet *p);

   /*
    * History management
    */
   
   double      PerHopTime(eamrp_rt_entry *rt);


        nsaddr_t        index;                  // IP Address of this node
        u_int32_t       seqno;                  // Sequence Number
        int             bid;                    // Broadcast ID

        eamrp_rtable         rthead;                 // routing table
        eamrp_ncache         nbhead;                 // Neighbor Cache
        eamrp_bcache          bihead;                 // Broadcast ID Cache

        /*
         * Timers
         */
        EAMRPBroadcastTimer  btimer;
        EAMRPHelloTimer      htimer;
        EAMRPNeighborTimer   ntimer;
        EAMRPRouteCacheTimer rtimer;
        EAMRPLocalRepairTimer lrtimer;

        /*
         * Routing Table
         */
        eamrp_rtable          rtable;
        /*
         *  A "drop-front" queue used by the routing layer to buffer
         *  packets to which it does not have a route.
         */
        eamrp_rqueue         rqueue;

        /*
         * A mechanism for logging the contents of the routing
         * table.
         */
        Trace           *logtarget;

        /*
         * A pointer to the network interface queue that sits
         * between the "classifier" and the "link layer".
         */
        PriQueue        *EAMRPifqueue;

        /*
         * Logging stuff
         */
        void            log_link_del(nsaddr_t dst);
        void            log_link_broke(Packet *p);
        void            log_link_kept(nsaddr_t dst);

			// EAMRP code
			int eamrp_max_paths_;
			int eamrp_prim_alt_path_len_diff_;
			// AODV ns-2.31 code
			/*	for passing packets up to agents */
			PortClassifier *dmux_;
};

#endif /* __eamrp_h__ */
