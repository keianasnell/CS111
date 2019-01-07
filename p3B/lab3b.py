import sys
import csv
import string

exit_status = 0

invalid_msg = "INVALID {} {} IN INODE {} AT OFFSET {}"
reserved_msg = "RESERVED {} {} IN INODE {} AT OFFSET {}"
duplicate_msg = "DUPLICATE {} {} IN INODE {} AT OFFSET {}"
dir_inode_msg = "DIRECTORY INODE {} NAME {} {} INODE {}"
link_count_msg = "INODE {} HAS {} LINKS BUT LINKCOUNT IS {}"
dir_parent_msg = "DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}"

class Superblock:
	def __init__(self, row):
		self.num_blocks = int(row[1])
		self.num_inodes = int(row[2])
		self.first_inode = int(row[7])

class Inode:
	def __init__(self, row):
		self.inode_num = int(row[1])
		self.link_count = int(row[6])
		self.blocks = [int(row[i]) for i in range(12,24)]
		self.indirects = [int(row[i]) for i in range(24,27)]
		
class Indirect:
	def __init__(self, row):
		self.owner = int(row[1])
		self.level = int(row[2])
		self.block_offset = int(row[3])
		self.ref_num = int(row[5])

class Dirent:
	def __init__(self, row):
		self.parent = int(row[1])
		self.inode = int(row[3])
		self.name = row[6]


def block_consistency(inodes, indirects, superblock, free_blocks):
	blocks = {}

	for inode in inodes:
		for offset, block in enumerate(inode.blocks):
			if block != 0:
				if block >= superblock.num_blocks or block < 0:
					print(invalid_msg.format("BLOCK", block, inode.inode_num, offset))
					exit_status = 2					
				elif block < 8:
					exit_status = 2
					print(reserved_msg.format("BLOCK", block, inode.inode_num, offset))
				elif block in blocks:
					blocks[block].append(("BLOCK", inode.inode_num, offset))
				else:
					blocks[block] = [("BLOCK", inode.inode_num, offset)]
			offset += 1
		
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
					print(invalid_msg.format(block_type, block, inode.inode_num, offset))
					exit_status = 2
				elif block < 8:
					print(reserved_msg.format(block_type, block, inode.inode_num, offset))
					exit_status = 2
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

		block = indirect.ref_num
		if block != 0:
			if block >= superblock.num_blocks or block < 0:
				print(invalid_msg.format(block_type, block, indirect.owner, indirect.block_offset))
				exit_status = 2
			elif block < 8:
				print(reserved_msg.format(block_type, block, indirect.owner, indirect.block_offset))
				exit_status = 2
			elif block in blocks:
				blocks[block].append((block_type, indirect.owner, offset))
			else:
				blocks[block] = [(block_type, indirect.owner, offset)]

	for block in range(8,superblock.num_blocks):
		if block not in free_blocks and block not in blocks:
			print("UNREFERENCED BLOCK {}".format(block))
			exit_status = 2
		elif block in free_blocks and block in blocks:
			print("ALLOCATED BLOCK {} ON FREELIST".format(block))
			exit_status = 2
		if block in blocks and len(blocks[block]) > 1:
			for block_type, inode, offset in blocks[block]:
				print(duplicate_msg.format(block_type, block, inode, offset))
				exit_status = 2


def inode_allocation(free_inodes, inodes, allocated_inodes, superblock):
	for inode in allocated_inodes:
		if inode in free_inodes:
			print("ALLOCATED INODE {} ON FREELIST".format(inode))
			exit_status = 2

	for i in range(superblock.first_inode, superblock.num_inodes):
		if i not in allocated_inodes and i not in free_inodes:
			print("UNALLOCATED INODE {} NOT ON FREELIST".format(i))
			exit_status = 2


def dir_consistency(inodes, allocated_inodes, dirents, superblock):
	link_count = [0] * (superblock.num_inodes + 1)
	parent = [0] * (superblock.num_inodes + 1)
	parent[2] = 2

	for dirent in dirents:
		if dirent.inode > superblock.num_inodes or dirent.inode < 1:
			print(dir_inode_msg.format(dirent.parent, dirent.name, "INVALID", dirent.inode))
			exit_status = 2
		elif dirent.inode not in allocated_inodes:
			print(dir_inode_msg.format(dirent.parent, dirent.name, "UNALLOCATED", dirent.inode))
			exit_status = 2
		else:
			link_count[dirent.inode] += 1
			if dirent.name != "'.'" and dirent.name != "'..'":
				parent[dirent.inode] = dirent.parent

	for inode in inodes:
		if inode.link_count != link_count[inode.inode_num]:
			print(link_count_msg.format(inode.inode_num, link_count[inode.inode_num], inode.link_count))
			exit_status = 2

	for dirent in dirents:
		if dirent.name == "'.'" and dirent.parent != dirent.inode:
			print(dir_parent_msg.format(dirent.parent, "'.'", dirent.inode, dirent.parent))
			exit_status = 2
		if dirent.name == "'..'" and parent[dirent.parent] != dirent.inode:
			print(dir_parent_msg.format(dirent.parent, "'..'", dirent.inode, dirent.parent))
			exit_status = 2


def main():
	if len(sys.argv) != 2:
		print("Error: Invalid arguments. Correct usage: ./lab3b CSV_FILENAME", file=sys.stderr)
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
	inode_allocation(free_inodes, inodes, allocated_inodes, superblock)
	dir_consistency(inodes, allocated_inodes, dirents, superblock)

	sys.exit(exit_status)

if __name__ == "__main__":
	main()
