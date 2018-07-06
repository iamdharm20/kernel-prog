#ifndef _dfs_types_h_
#define _dfs_types_h_

#define dfs_module_name 	"dfs_test"
#define dfs_dev_cpu_name	"dfs_cpu"
#define dfs_dev_mem_name	"dfs_mem"

struct dfs_pack {
	char		symbol;		//magic number
	int		type;
	int 		status;
};




#endif
