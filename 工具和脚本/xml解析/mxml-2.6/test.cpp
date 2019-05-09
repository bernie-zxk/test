#include <iostream>
#include <cassert>
#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "mxml.h"

using namespace std;
mxml_node_t *mxml_open(const char *filename);
const char *GetAttrValue(mxml_node_t *tree, const char *ind, const char *name);

int main(int argc, char **argv)
{
	mxml_node_t *tree = mxml_open(argv[1]);
	
	const char *com_svr = GetAttrValue(tree, "COM_SVR", "name");	
	cout << com_svr << endl;
		

}

mxml_node_t *mxml_open(const char *filename)
{
		mxml_node_t *tree;
		int fd = open(filename, O_RDONLY);
		if ( fd == -1 ) {
			perror("open:");	
			exit(1);
		}

		tree = mxmlLoadFd(NULL, fd, NULL);
		if ( !tree ) {
			fprintf(stderr, "mxmlLoaFd failed!\n");	
			exit(1);	
		}
		close(fd);

		return tree;
}

const char *GetAttrValue(mxml_node_t *tree, const char *ind, const char *name)
{
		mxml_node_t *node = mxmlFindElement(tree, tree, ind, name, NULL, MXML_DESCEND);
		const char *value = NULL;
		if ( node ) {
			value = mxmlElementGetAttr(node, name);
		}
		else
				return NULL;

		return value;
}
