


#include <eamrp/eamrp_rtable.h>
//#include <cmu/aodv/aodv.h>

/*
  The Routing Table
*/

eamrp_rt_entry::eamrp_rt_entry()
{
int i;

 rt_req_timeout = 0.0;
 rt_req_cnt = 0;

 rt_dst = 0;
 rt_seqno = 0;
 // EAMRP code
 rt_last_hop_count = INFINITY2;
 rt_advertised_hops = INFINITY2;
 LIST_INIT(&rt_path_list);
 rt_highest_seqno_heard = 0;
 rt_num_paths_ = 0;
 rt_error = false;
 
 LIST_INIT(&rt_pclist);
 rt_expire = 0.0;
 rt_flags = RTF_DOWN;

 /*
 rt_errors = 0;
 rt_error_time = 0.0;
 */


 for (i=0; i < MAX_HISTORY; i++) {
   rt_disc_latency[i] = 0.0;
 }
 hist_indx = 0;
 rt_req_last_ttl = 0;

 LIST_INIT(&rt_nblist);

}


eamrp_rt_entry::~eamrp_rt_entry()
{
EAMRP_Neighbor *nb;

 while((nb = rt_nblist.lh_first)) {
   LIST_REMOVE(nb, nb_link);
   delete nb;
 }

 // EAMRP code
EAMRP_Path *path;

 while((path = rt_path_list.lh_first)) {
   LIST_REMOVE(path, path_link);
   delete path;
 }
 
EAMRP_Precursor *pc;

 while((pc = rt_pclist.lh_first)) {
   LIST_REMOVE(pc, pc_link);
   delete pc;
 }

}


void
eamrp_rt_entry::nb_insert(nsaddr_t id)
{
EAMRP_Neighbor *nb = new EAMRP_Neighbor(id);
        
 assert(nb);
 nb->nb_expire = 0;
 LIST_INSERT_HEAD(&rt_nblist, nb, nb_link);
printf("<#message#>");
}


EAMRP_Neighbor*
eamrp_rt_entry::nb_lookup(nsaddr_t id)
{
EAMRP_Neighbor *nb = rt_nblist.lh_first;

 for(; nb; nb = nb->nb_link.le_next) {
   if(nb->nb_addr == id)
     break;
 }
 return nb;

}

// EAMRP code
EAMRP_Path*
eamrp_rt_entry::path_insert(nsaddr_t nexthop, u_int16_t hopcount, double expire_time, nsaddr_t lasthop) {
EAMRP_Path *path = new EAMRP_Path(nexthop, hopcount, expire_time, lasthop);
        
   assert(path);
#ifdef DEBUG
   fprintf(stderr, "%s: (%d\t%d)\n", __FUNCTION__, path->nexthop, path->hopcount);
#endif // DEBUG

   /*
    * Insert path at the end of the list
    */
EAMRP_Path *p = rt_path_list.lh_first;
   if (p) {
      for(; p->path_link.le_next; p = p->path_link.le_next) 
	/* Do nothing */;
      LIST_INSERT_AFTER(p, path, path_link);
   }
   else {
      LIST_INSERT_HEAD(&rt_path_list, path, path_link);
   }
   rt_num_paths_ += 1;

   return path;
}

EAMRP_Path*
eamrp_rt_entry::path_lookup(nsaddr_t id)
{
EAMRP_Path *path = rt_path_list.lh_first;

 for(; path; path = path->path_link.le_next) {
   if (path->nexthop == id)
      return path;
 }
 return NULL;

}

EAMRP_Path*
eamrp_rt_entry::disjoint_path_lookup(nsaddr_t nexthop, nsaddr_t lasthop)
{
EAMRP_Path *path = rt_path_list.lh_first;

 for(; path; path = path->path_link.le_next) {
   if ( (path->nexthop == nexthop) && (path->lasthop == lasthop) )
      return path;
 }
 return NULL;

}

/* Returns true if no path exists (for this route entry) which has 'nexthop' as next hop or 'lasthop' as last hop.*/
bool
eamrp_rt_entry::new_disjoint_path(nsaddr_t nexthop, nsaddr_t lasthop)
{
  EAMRP_Path *path = rt_path_list.lh_first;

  for(; path; path = path->path_link.le_next) {
    if ( (path->nexthop == nexthop) || (path->lasthop == lasthop) )
      return false;
  }
  return true;

}


EAMRP_Path*
eamrp_rt_entry::path_lookup_lasthop(nsaddr_t id)
{
EAMRP_Path *path = rt_path_list.lh_first;

 for(; path; path = path->path_link.le_next) {
   if (path->lasthop == id)
      return path;
 }
 return NULL;

}


void
eamrp_rt_entry::path_delete(nsaddr_t id) {
EAMRP_Path *path = rt_path_list.lh_first;

 for(; path; path = path->path_link.le_next) {
   if(path->nexthop == id) {
     LIST_REMOVE(path,path_link);
     delete path;
     rt_num_paths_ -= 1;
     break;
   }
 }

}

void
eamrp_rt_entry::path_delete(void) {
EAMRP_Path *path;

 while((path = rt_path_list.lh_first)) {
   LIST_REMOVE(path, path_link);
   delete path;
 }
 rt_num_paths_ = 0;
}  

void
eamrp_rt_entry::path_delete_longest(void) {
EAMRP_Path *p = rt_path_list.lh_first;
EAMRP_Path *path = NULL;
u_int16_t max_hopcount = 0;

 for(; p; p = p->path_link.le_next) {
   if(p->hopcount > max_hopcount) {
      assert (p->hopcount != INFINITY2);
      path = p;
      max_hopcount = p->hopcount;
   }
 }

 if (path) {
     LIST_REMOVE(path, path_link);
     delete path;
     rt_num_paths_ -= 1;
 }
}

bool
eamrp_rt_entry::path_empty(void) {
EAMRP_Path *path;

 if ((path = rt_path_list.lh_first)) {
    assert (rt_num_paths_ > 0);
    return false;
 }
 else {  
    assert (rt_num_paths_ == 0);
    return true;
 }
}  

EAMRP_Path*
eamrp_rt_entry::path_findMinHop(void)
{
EAMRP_Path *p = rt_path_list.lh_first;
EAMRP_Path *path = NULL;
u_int16_t min_hopcount = 0xffff;

 for (; p; p = p->path_link.le_next) {
   if (p->hopcount < min_hopcount) {
      path = p;
      min_hopcount = p->hopcount;
   }
 }

 return path;
}

EAMRP_Path*
eamrp_rt_entry::path_find(void) {
EAMRP_Path *p = rt_path_list.lh_first;

   return p;
}

u_int16_t
eamrp_rt_entry::path_get_max_hopcount(void)
{
EAMRP_Path *path = rt_path_list.lh_first;
u_int16_t max_hopcount = 0;

 for(; path; path = path->path_link.le_next) {
   if(path->hopcount > max_hopcount) {
      max_hopcount = path->hopcount;
   }
 }
 if (max_hopcount == 0) return INFINITY2;
 else return max_hopcount;
}

u_int16_t
eamrp_rt_entry::path_get_min_hopcount(void)
{
EAMRP_Path *path = rt_path_list.lh_first;
u_int16_t min_hopcount = INFINITY2;

 for(; path; path = path->path_link.le_next) {
   if(path->hopcount < min_hopcount) {
      min_hopcount = path->hopcount;
   }
 }
 return min_hopcount;
}

double
eamrp_rt_entry::path_get_max_expiration_time(void) {
EAMRP_Path *path = rt_path_list.lh_first;
double max_expire_time = 0;

 for(; path; path = path->path_link.le_next) {
   if(path->expire > max_expire_time) {
      max_expire_time = path->expire;
   }
 }
 return max_expire_time;
}

void
eamrp_rt_entry::path_purge(void) {
double now = Scheduler::instance().clock();
bool cond;

 do {
 EAMRP_Path *path = rt_path_list.lh_first;
  cond = false;
  for(; path; path = path->path_link.le_next) {
    if(path->expire < now) {
      cond = true;
      LIST_REMOVE(path, path_link);
      delete path;
      rt_num_paths_ -= 1;
      break;
    }
  }
 } while (cond);
}

void
eamrp_rt_entry::pc_insert(nsaddr_t id)
{
	if (pc_lookup(id) == NULL) {
	EAMRP_Precursor *pc = new EAMRP_Precursor(id);
        
 		assert(pc);
 		LIST_INSERT_HEAD(&rt_pclist, pc, pc_link);
	}
}


EAMRP_Precursor*
eamrp_rt_entry::pc_lookup(nsaddr_t id)
{
EAMRP_Precursor *pc = rt_pclist.lh_first;

 for(; pc; pc = pc->pc_link.le_next) {
   if(pc->pc_addr == id)
   	return pc;
 }
 return NULL;

}

void
eamrp_rt_entry::pc_delete(nsaddr_t id) {
EAMRP_Precursor *pc = rt_pclist.lh_first;

 for(; pc; pc = pc->pc_link.le_next) {
   if(pc->pc_addr == id) {
     LIST_REMOVE(pc,pc_link);
     delete pc;
     break;
   }
 }

}

void
eamrp_rt_entry::pc_delete(void) {
EAMRP_Precursor *pc;

 while((pc = rt_pclist.lh_first)) {
   LIST_REMOVE(pc, pc_link);
   delete pc;
 }
}	

bool
eamrp_rt_entry::pc_empty(void) {
EAMRP_Precursor *pc;

 if ((pc = rt_pclist.lh_first)) return false;
 else return true;
}	

/*
  The Routing Table
*/

eamrp_rt_entry*
eamrp_rtable::rt_lookup(nsaddr_t id)
{
eamrp_rt_entry *rt = rthead.lh_first;

 for(; rt; rt = rt->rt_link.le_next) {
   if(rt->rt_dst == id)
     break;
 }
 return rt;

}

void
eamrp_rtable::rt_delete(nsaddr_t id)
{
eamrp_rt_entry *rt = rt_lookup(id);

 if(rt) {
   LIST_REMOVE(rt, rt_link);
   delete rt;
 }

}

eamrp_rt_entry*
eamrp_rtable::rt_add(nsaddr_t id)
{
eamrp_rt_entry *rt;

 assert(rt_lookup(id) == 0);
 rt = new eamrp_rt_entry;
 assert(rt);
 rt->rt_dst = id;
 LIST_INSERT_HEAD(&rthead, rt, rt_link);
 return rt;
}

// EAMRP code
void eamrp_rtable::rt_dumptable() {
  eamrp_rt_entry *rt = rthead.lh_first;
  while(rt != 0) {
    printf("%6s  %6s  ", "Dest", "Seq#");
    printf("%6s  %6s  %6s  %6s\n", "Advhop", "Nxthop", "Hopcnt", "Lsthop");
    printf("%6d  %6d  ", rt->rt_dst, rt->rt_seqno);  
    printf("%6d\n", rt->rt_advertised_hops);
    /* Print path list for this route entry. */
    EAMRP_Path *paths = rt->rt_path_list.lh_first;
    paths->printPaths();
    printf("\n");
    rt = rt->rt_link.le_next;
  }
}

// EAMRP code
bool eamrp_rtable::rt_has_active_route() {
  /* Go through list of route entries to see if there exists 
     any valid route entry */
  eamrp_rt_entry *rt = rthead.lh_first;
  while(rt != 0) {
    if (rt->rt_flags == RTF_UP)
      return true;
    rt = rt->rt_link.le_next;
  }
  return false;
}

