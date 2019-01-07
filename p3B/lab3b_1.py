import sys
import csv
import string

exit_status = 0
invalid_block = "INVALID {} {} IN INODE {} AT OFFSET {}"
reserved_block = "RESERVED {} {} IN INODE {} AT OFFSET {}"

allocated_block = "ALLOCATED BLOCK {} ON FREELIST"
unreferenced_block = "UNREFERENCED BLOCK {}"
duplicate_block = "DUPLICATE {} {} IN INODE {} AT OFFSET {}"

class Superblock:
	def __init__(self, data):
		self.num_blocks = int(data[1])
		self.num_inodes = int(data[2])
		self.block_size = int(data[3])
		self.inode_size = int(data[4])
		self.blocks_per_group = int(data[5])
		self.inodes_per_group = int(data[6])
		self.first_inode = int(data[7])

class Inode:
	def __init__(self, data):
		self.inode_num = int(data[1])
		self.file_type = data[2]
		self.mode = data[3]
		self.owner = int(data[4])
		self.group = int(data[5])
		self.link_count = int(data[6])
		self.file_size = int(data[10])
		self.num_blocks = int(data[11])
		self.blocks = [int(data[i]) for i in range(12,24)]
		self.indirects = [int(data[i]) for i in range(24,27)]

class Dirent:
	def __init__(self, data):
		self.parent = int(data[1])
		self.offset = int(data[2])
		self.inode = int(data[3])
		self.length_rec = int(data[4])
		self.length_name = int(data[5])
		self.name = data[6]

class Indirect:
	def __init__(self, data):
		self.inode_owner = int(data[1])
		self.level = int(data[2])
		self.b_offset = int(data[3])
		self.block_num = int(data[4])
		self.block_ref = int(data[5])


def block_consistency(inodes, indirects, superblock, free_blocks):
	blocks = {}
	for inode in inodes:
		for offset, block in enumerate(inode.blocks):
			if block != 0:
			
				m = ("BLOCK", inode.inode_num, offset)
				if block < 0 or block >= superblock.num_blocks:
					exit_status = 2
					print(invalid_block.format("BLOCK", block, inode.inode_num, offset))
				elif block < 8:
					exit_status = 2
					print(reserved_block.format("BLOCK", block, inode.inode_num, offset))
				elif block in blocks:
					blocks[block].append(m)
				else:
					blocks[block] = [m]


		block_type = None
		for i in range(3):
			if i == 0:
				block_type = "INDIRECT BLOCK"
				offset = 12
			elif i == 1:
				block_type = "DOUBLE INDIRECT BLOCK"
				offset = 268
			elif i == 2:
				block_type = "TRIPLE INDIRECT BLOCK"
				offset = 65804
			block = inode.indirects[i]
			if block != 0:
				if block >= superblock.num_blocks or block < 0:
					exit_status = 2
					print(invalid_block.format(block_type, block, inode.inode_num, offset))
				elif block < 8:
					exit_status = 2
					print(reserved_block.format(block_type, block, inode.inode_num, offset))
				elif block in blocks:
					blocks[block].append((block_type, inode.inode_num, offset))
				else:
					blocks[block] = [(block_type, inode.inode_num, offset)]


	for indirect in indirects:
		block_type = None
		if indirect.level == 1:
			block_type = "BLOCK"
		elif indirect.level == 2:
			block_type = "INDIRECT BLOCK"
		elif indirect.level == 3:
			block_type = "DOUBLE INDIRECT BLOCK"
			block = indirect.block_ref
		if block != 0:
			if block >= superblock.num_blocks or block < 0:
				exit_status = 2
				print(invalid_block.format(block_type, block, indirect.inode_owner, indirect.b_offset))
			elif block < 8:
				exit_status = 2
				print(reserved_block.format(block_type, block, indirect.inode_owner, indirect.b_offset))
			elif block in blocks:
				blocks[block].append((block_type, indirect.inode_owner, offset))
			else:
				blocks[block] = [(block_type, indirect.inode_owner, offset)]


	for block in range(8,superblock.num_blocks):
		if block not in free_blocks and block not in blocks:
			exit_status = 2
			print("UNREFERENCED BLOCK {}".format(block))
		elif block in free_blocks and block in blocks:
			exit_status = 2
			print("ALLOCATED BLOCK {} ON FREELIST".format(block))
		if block in blocks and len(blocks[block]) > 1:
			for block_type, inode, offset in blocks[block]:
				exit_status = 2
				print(duplicate_block.format(block_type, block, inode, offset))



def dir_consistency(inodes, allocated_inodes, dirents, superblock):
	link_count = [0] * (superblock.num_inodes + 1)
	parent = [0] * (superblock.num_inodes + 1)
	parent[2] = 2


	for dirent in dirents:
		if dirent.inode > superblock.num_inodes or dirent.inode < 1:
			exit_status = 2
			print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(dirent.parent, dirent.name, dirent.inode))
		elif dirent.inode not in allocated_inodes:
			exit_status = 2
			print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(dirent.parent, dirent.name, dirent.inode))
		else:
			link_count[dirent.inode] += 1
			if dirent.name != "'.'" and dirent.name != "'..'":
				parent[dirent.inode] = dirent.parent

	for inode in inodes:
		if inode.link_count != link_count[inode.inode_num]:
			exit_status = 2
			print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.inode_num, link_count[inode.inode_num], inode.link_count))

	for dirent in dirents:
		if dirent.name == "'.'" and dirent.parent != dirent.inode:
			exit_status = 2
			print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(dirent.parent_inode_num, dirent.inode, dirent.parent))
		if dirent.name == "'..'" and parent[dirent.parent] != dirent.inode:
			exit_status = 2
			print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(dirent.parent, dirent.inode, dirent.parent))



def inode_allocation(allocated_inodes, free_inodes, superblock):
	for inode in allocated_inodes:
		if inode in free_inodes:
			exit_status = 2
			print("ALLOCATED INODE {} ON FREELIST".format(inode))

	for i in range(superblock.first_inode, superblock.num_inodes):
		if i not in allocated_inodes and i not in free_inodes:
			exit_status = 2
			print("UNALLOCATED INODE {} NOT ON FREELIST".format(i))


def main():
	if len(sys.argv) != 2:
		print("Error: Invalid arguments. Correct arguments: ./lab3b __filename__", file=sys.stderr)
		sys.exit(1)
		
 
	free_blocks = []
	free_inodes = []
	inodes = []
	dirents = []
	indirects = []

	try:
		with open(sys.argv[1], 'r') as csvfile:
			read = csv.reader(csvfile)
			for line in read:
				curr_line = line[0]
				if curr_line == 'SUPERBLOCK':
					superblock = Superblock(line)
				elif curr_line == 'BFREE':
					free_blocks.append(int(line[1]))
				elif curr_line == 'IFREE':
					free_inodes.append(int(line[1]))
				elif curr_line == 'INODE':
					inodes.append(Inode(line))
				elif curr_line == 'DIRENT':
					dirents.append(Dirent(line))
				elif curr_line == 'INDIRECT':
					indirects.append(Indirect(line))
	except (IOError, OSError):
		print("Error in reading file; no such file or directory exists", file=sys.stderr)
		sys.exit(1)

	allocated_inodes = set([inode.inode_num for inode in inodes])
	block_consistency(inodes, indirects, superblock, free_blocks)
	inode_allocation(allocated_inodes, dirents, superblock)
	dir_consistency(inodes, allocated_inodes, dirents, superblock)


	sys.exit(exit_status)



if __name__ == "__main__":
	main()

