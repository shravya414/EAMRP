


#ifndef __eamrp_rtable_h__
#define __eamrp_rtable_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

#define CURRENT_TIME    Scheduler::instance().clock()
#define INFINITY2        0xff

/*
   EAMRP Neighbor Cache Entry
*/
class EAMRP_Neighbor {
        friend class EAMRP;
        friend class eamrp_rt_entry;
 public:
        EAMRP_Neighbor(u_int32_t a) { nb_addr = a; }

protected:
        LIST_ENTRY(EAMRP_Neighbor) nb_link;
        nsaddr_t        nb_addr;
        double          nb_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
};

LIST_HEAD(eamrp_ncache, EAMRP_Neighbor);

 // EAMRP code
/*
   EAMRP Path list data structure
*/
class EAMRP_Path {
        friend class EAMRP;
        friend class eamrp_rt_entry;
 public:
        EAMRP_Path(nsaddr_t nh, u_int16_t h, double expire_time, nsaddr_t lh=0) { 
           nexthop = nh;
           hopcount = h;
           expire = expire_time;
           ts = Scheduler::instance().clock();
           lasthop = lh;
           // CHANGE
           error = false;
           // CHANGE    
        }
	void printPath() { 
	  printf("                        %6d  %6d  %6d\n", nexthop, hopcount, lasthop);
	}
	void printPaths() {
	  EAMRP_Path *p = this;
	  for (; p; p = p->path_link.le_next) {
	    p->printPath();
	  }
	}

 protected:
        LIST_ENTRY(EAMRP_Path) path_link;
        nsaddr_t        nexthop;    // nexthop address
        u_int16_t       hopcount;   // hopcount through this nexthop
        double          expire;     // expiration timeout
        double          ts;         // time when we saw this nexthop
        nsaddr_t        lasthop;    // lasthop address
        // CHANGE
        bool            error;
        // CHANGE
};

LIST_HEAD(eamrp_paths, EAMRP_Path);

/*
   EAMRP Precursor list data structure
*/
class EAMRP_Precursor {
        friend class EAMRP;
        friend class eamrp_rt_entry;
 public:
        EAMRP_Precursor(u_int32_t a) { pc_addr = a; }
		  
 protected:
        LIST_ENTRY(EAMRP_Precursor) pc_link;
        nsaddr_t        pc_addr;	// precursor address
};

LIST_HEAD(eamrp_precursors, EAMRP_Precursor);


/*
  Route Table Entry
*/

class eamrp_rt_entry {
        friend class eamrp_rtable;
        friend class EAMRP;
	friend class LocalRepairTimer;
 public:
        eamrp_rt_entry();
        ~eamrp_rt_entry();

        void            nb_insert(nsaddr_t id);
        EAMRP_Neighbor*  nb_lookup(nsaddr_t id);

 // EAMRP code
        EAMRP_Path*   path_insert(nsaddr_t nexthop, u_int16_t hopcount, double expire_time, nsaddr_t lasthop=0);

        EAMRP_Path*   path_lookup(nsaddr_t id);  // lookup path by nexthop

        EAMRP_Path*   disjoint_path_lookup(nsaddr_t nexthop, nsaddr_t lasthop);
        bool         new_disjoint_path(nsaddr_t nexthop, nsaddr_t lasthop);

        EAMRP_Path*   path_lookup_lasthop(nsaddr_t id);   // lookup path by lasthop
        void         path_delete(nsaddr_t id);           // delete path by nexthop
        void         path_delete(void);                  // delete all paths
        void         path_delete_longest(void);          // delete longest path
        bool         path_empty(void);                   // is the path list empty?
        EAMRP_Path*   path_find(void);                    // find the path that we got first
        EAMRP_Path*   path_findMinHop(void);              // find the shortest path
        u_int16_t    path_get_max_hopcount(void);  
        u_int16_t    path_get_min_hopcount(void);  
        double       path_get_max_expiration_time(void); 
        void         path_purge(void);
        void            pc_insert(nsaddr_t id);
        EAMRP_Precursor* pc_lookup(nsaddr_t id);
        void 		pc_delete(nsaddr_t id);
        void 		pc_delete(void);
        bool 		pc_empty(void);

        double          rt_req_timeout;         // when I can send another req
        u_int8_t        rt_req_cnt;             // number of route requests
	
		  // EAMRP code
        u_int8_t        rt_flags;
 protected:
        LIST_ENTRY(eamrp_rt_entry) rt_link;

        nsaddr_t        rt_dst;
        u_int32_t       rt_seqno;
	/* u_int8_t 	rt_interface; */
 // EAMRP code
        u_int16_t       rt_hops;             // hop count
        u_int16_t       rt_advertised_hops;  // advertised hop count
	int 		rt_last_hop_count;	// last valid hop count
 // EAMRP code
        eamrp_paths      rt_path_list;     // list of paths
        u_int32_t       rt_highest_seqno_heard; 
        int             rt_num_paths_;
	bool rt_error;
        
	/* list of precursors */ 
        eamrp_precursors rt_pclist;
        double          rt_expire;     		// when entry expires

#define RTF_DOWN 0
#define RTF_UP 1
#define RTF_IN_REPAIR 2

        /*
         *  Must receive 4 errors within 3 seconds in order to mark
         *  the route down.
        u_int8_t        rt_errors;      // error count
        double          rt_error_time;
#define MAX_RT_ERROR            4       // errors
#define MAX_RT_ERROR_TIME       3       // seconds
         */

#define MAX_HISTORY	3
	double 		rt_disc_latency[MAX_HISTORY];
	char 		hist_indx;
        int 		rt_req_last_ttl;        // last ttl value used
	// last few route discovery latencies
	// double 		rt_length [MAX_HISTORY];
	// last few route lengths

        /*
         * a list of neighbors that are using this route.
         */
        eamrp_ncache          rt_nblist;
};


/*
  The Routing Table
*/

class eamrp_rtable {
 public:
	eamrp_rtable() { LIST_INIT(&rthead); }

        eamrp_rt_entry*       head() { return rthead.lh_first; }

        eamrp_rt_entry*       rt_add(nsaddr_t id);
        void                 rt_delete(nsaddr_t id);
        eamrp_rt_entry*       rt_lookup(nsaddr_t id);
 // EAMRP code
	void                 rt_dumptable();
	bool                 rt_has_active_route();

 private:
        LIST_HEAD(eamrp_rthead, eamrp_rt_entry) rthead;
};

#endif /* _eamrp__rtable_h__ */
