#include"mode_obj.h"
#include"TAG_BLOCK_structures.h"

dfbt_list* List_block_headers(char* tag_data, unsigned int size)
{
	//first dfbt starts at 0x40
	dfbt* my_dfbt = (dfbt*)((char*)tag_data + 0x40);

	dfbt_list* ret = new dfbt_list();
	ret->DFBT = my_dfbt;
	ret->next_header = nullptr;

	dfbt_list* current_header_in_action = ret;

	for (int i = 0x50; i < size; i++)
	{
		if (*(tag_data + i) == DFBT[0])
		{
			if (*(tag_data + i + 1) == DFBT[1])
			{
				if (*(tag_data + i + 2) == DFBT[2])
				{
					if (*(tag_data + i + 3) == DFBT[3])
					{
						//well found another dfbt
						dfbt* my_dfbt = (dfbt*)((char*)tag_data + i);

						dfbt_list* temp = new dfbt_list();
						temp->DFBT = my_dfbt;
						temp->next_header = nullptr;

						current_header_in_action->next_header = temp;

						current_header_in_action = temp;
					}
				}
			}
		}
	}
	return ret;
}
dfbt* Get_dfbt_from_size(dfbt_list* list, unsigned int block_size, char* mem_start)
{
	while (list)
	{
		if ((list->DFBT->block_size == block_size) && (((char*)list->DFBT) > mem_start))
			return list->DFBT;

		list = list->next_header;
	}
	return nullptr;
}
string Get_string(char* mem, int length)
{
	string ret = "";

	while (length--)
		ret += *(char*)(mem++);
	
	return ret;
}
section_data::section_data(){}
section_data::section_data(dfbt* header_ptr, dfbt_list* my_dfbt_list,bool uncompressed)
{
	
	access_index = 0;

	//time to read the count
	char* section_data = (char*)header_ptr + 0x10;
	unsigned int parts_count = *(unsigned int*)(section_data);//off:0x0
	///IDK what is its use
	//unsigned int subparts_count = *(section_data + 0xC);//off:0xc
	unsigned int RAW_vertex_count = *(unsigned int*)(section_data + 0x24);
	unsigned int strip_index_count = *(unsigned int*)(section_data + 0x30);
	unsigned int Node_map_count = *(unsigned int*)(section_data + 0xA4);

	//load the parts shit
	char* mem_start = (char*)header_ptr;
	if (parts_count > 0)
	{
		dfbt* parts_header = Get_dfbt_from_size(my_dfbt_list, 0x48, mem_start);
		char* parts_dat_base = (char*)parts_header + 0x10;

		for (int i = 0; i < parts_count; i++)
		{
			char* parts_dat = parts_dat_base + i * 0x48;
			parts temp;

			temp.material_index = *(__int16*)(parts_dat + 0x4);
			temp.start_index = *(__int16*)(parts_dat + 0x6);
			temp.length = *(__int16*)(parts_dat + 0x8);

			temp.centroid = *(vector3d*)(parts_dat + 0x10);


			for (int i = 0; i < 4; i++)
			{
				temp.nodes[i].index = *(__int8*)(parts_dat + 0x1C + i * 0x4);
				temp.nodes[i].weight = *(float*)(parts_dat + 0x2C + i * 0x4);
			}
			//add to the list
			parts_list.push_back(temp);
		}
		mem_start = (char*)parts_header;
	}
	//load the RAW_vertex_stuff
	if (RAW_vertex_count > 0)
	{
		dfbt* RAW_vertex_header = Get_dfbt_from_size(my_dfbt_list, 0xC4, mem_start);
		char* RAW_vertex_base = (char*)RAW_vertex_header + 0x10;

		for (int i = 0; i < RAW_vertex_count; i++)
		{
			char* RAW_vertex_dat = RAW_vertex_base + i * 0xC4;
			RAW_vertex temp;
			
			temp.pos.x = *(float*)(RAW_vertex_dat + 0x0);
			temp.pos.y = *(float*)(RAW_vertex_dat + 0x4);
			temp.pos.z = *(float*)(RAW_vertex_dat + 0x8);

			for (int i = 0; i < 4; i++)
			{
				temp.nodes[i].index = *(__int8*)(RAW_vertex_dat + 0x2C + i * 0x4);
				temp.nodes[i].weight = *(float*)(RAW_vertex_dat + 0x1C + i * 0x4);
			}

			//*(int*)(RAW_vertex_dat + 0x3C) = 0x1;

			temp.tex_cord.x = *(float*)(RAW_vertex_dat + 0x44);
			temp.tex_cord.y = 1 - *(float*)(RAW_vertex_dat + 0x48);						//<--------some UV crap on its way

			temp.normal.x = *(float*)(RAW_vertex_dat + 0x4C);
			temp.normal.y = *(float*)(RAW_vertex_dat + 0x50);
			temp.normal.z = *(float*)(RAW_vertex_dat + 0x54);

			temp.binormal.x = *(float*)(RAW_vertex_dat + 0x58);
			temp.binormal.y = *(float*)(RAW_vertex_dat + 0x5C);
			temp.binormal.z = *(float*)(RAW_vertex_dat + 0x60);

			temp.tangent.x = *(float*)(RAW_vertex_dat + 0x64);
			temp.tangent.y = *(float*)(RAW_vertex_dat + 0x68);
			temp.tangent.z = *(float*)(RAW_vertex_dat + 0x6C);

			temp.anisotropic_binormal.x = *(float*)(RAW_vertex_dat + 0x70);
			temp.anisotropic_binormal.y = *(float*)(RAW_vertex_dat + 0x74);
			temp.anisotropic_binormal.z = *(float*)(RAW_vertex_dat + 0x78);

			//add to the list
			vertex_list.push_back(temp);
		}
		mem_start = (char*)RAW_vertex_header;
	}
	//Load the strip index data
	if (strip_index_count > 0)
	{
		dfbt* strip_index_header = Get_dfbt_from_size(my_dfbt_list, 0x2, mem_start);
		char* strip_index_base = (char*)strip_index_header + 0x10;

		for (int i = 0; i < strip_index_count; i++)
		{
			char* strip_index_dat = strip_index_base + i * 0x2;
			strip_index temp;

			temp.vertex_index = *(__int16*)(strip_index_dat + 0x0);

			strip_index_list.push_back(temp);
		}
		mem_start = (char*)strip_index_header;
	}
	//Load node_map data
	if (Node_map_count > 0)
	{
		dfbt* node_map_header = Get_dfbt_from_size(my_dfbt_list, 0x1, mem_start);
		char* node_map_base = (char*)node_map_header + 0x10;

		for (int i = 0; i < Node_map_count; i++)
		{
			char* node_map_dat = node_map_base + i * 0x1;
			node_map temp;

			temp.node_index = *(__int8*)(node_map_dat + 0x0);

			node_map_list.push_back(temp);
		}
		mem_start = (char*)node_map_header;
	}
	/*
	i wrote the above parsing codes under the assumption 
	that some of the blocks arent used(usually)
	*/

	Generate_Faces(uncompressed);
}
section_data::~section_data(){}
void section_data::Generate_Faces(bool uncompressed)
{
	if (!uncompressed)
	{
		//final logic
		int index_start_offset = 0;
		for (int i = 0; i < parts_list.size(); i++)
		{
			int index_buffer_size = parts_list[i].length;//size per part

			if (index_buffer_size >= 3)
			{
				for (int j = 0; j < index_buffer_size - 2; j++)
				{
					triangle_face t_face;
					t_face.v0 = strip_index_list[index_start_offset + j].vertex_index;

					if (j % 2 == 0)
					{
						t_face.v1 = strip_index_list[index_start_offset + j + 1].vertex_index;
						t_face.v2 = strip_index_list[index_start_offset + j + 2].vertex_index;
					}
					else
					{
						t_face.v2 = strip_index_list[index_start_offset + j + 1].vertex_index;
						t_face.v1 = strip_index_list[index_start_offset + j + 2].vertex_index;
					}
					if (t_face.v0 != t_face.v1&&t_face.v1 != t_face.v2&&t_face.v2 != t_face.v0)
						parts_list[i].face_list.push_back(t_face);
				}
			}
			index_start_offset += index_buffer_size;
		}
	}
	else
	{
		for (int k = 0; k < parts_list.size(); k++)
		{
			int indexCount = parts_list[k].length;
			int start_index = parts_list[k].start_index;

			if (indexCount >= 3)
			{
				for (int i = 0; i < (indexCount - 2); i+=3)
				{
					triangle_face temp;

					temp.v0 = strip_index_list[start_index + i].vertex_index;
					temp.v1 = strip_index_list[start_index + i + 1].vertex_index;
					temp.v2 = strip_index_list[start_index + i + 2].vertex_index;

					parts_list[k].face_list.push_back(temp);
				}
			}
		}
	}
}

mode::mode(tag_data_struct& arg0)
{
	mode_data = arg0.tag_data;
	mode_size = arg0.size;

	char* mem_start = (char*)mode_data;
	//load section and its child blocks
	dfbt_list* my_dfbt_list = List_block_headers(mode_data, mode_size);
	dfbt* mode_header = Get_dfbt_from_size(my_dfbt_list, 0xB8, mem_start);

	dfbt* regions_header = Get_dfbt_from_size(my_dfbt_list, 0x14, mem_start);

	//read name
	char* mode_header_mem = (char*)mode_header + 0x10;
	__int8 name_length = *(__int8*)(mode_header_mem + 0x3);

	mem_start = (char*)mode_header;
	name = Get_string((char*)regions_header - name_length, name_length);
	///
	//-----------------------------LOADING REGIONS AND PERMUTATIONS---------------------------------------------
	///
	
	mem_start = (char*)regions_header;

	unsigned int regions_count = regions_header->block_count;
	char* regions_header_mem = (char*)regions_header + 0x10;

	while (regions_count--)
	{
		__int8 region_name_length = *(__int8*)(regions_header_mem + 0x3);
		int permutations_count = *(int*)(regions_header_mem + 0x8);

		//----------------------------AT least ONE Permutation per region is required 
		//TIME TO LOAD THE PERMS
		if (permutations_count)
		{
			regions temp_region;
			char* string_ptr;

			dfbt* permutation_header = Get_dfbt_from_size(my_dfbt_list, 0x10, mem_start);
			char* permutation_header_mem = (char*)permutation_header + 0x10;

			temp_region.name = Get_string((char*)permutation_header - region_name_length, region_name_length);
			string_ptr = permutation_header_mem + permutations_count * 0x10;

			///load up permutation data
			while (permutations_count--)
			{
				__int8 permutation_name_length = *(__int8*)(permutation_header_mem + 0x3);

				permutations temp_permutations;

				temp_permutations.L1 = *(__int16*)(permutation_header_mem + 0x4);
				temp_permutations.L2 = *(__int16*)(permutation_header_mem + 0x4 + 0x2);
				temp_permutations.L3 = *(__int16*)(permutation_header_mem + 0x4 + 0x4);
				temp_permutations.L4 = *(__int16*)(permutation_header_mem + 0x4 + 0x6);
				temp_permutations.L5 = *(__int16*)(permutation_header_mem + 0x4 + 0x8);
				temp_permutations.L6 = *(__int16*)(permutation_header_mem + 0x4 + 0xA);

				temp_permutations.name = Get_string(string_ptr, permutation_name_length);

				temp_region.perms.push_back(temp_permutations);

				string_ptr += permutation_name_length;
				permutation_header_mem += 0x10;
			}

			region_list.push_back(temp_region);

			mem_start = (char*)permutation_header;
		}	
		regions_header_mem += 0x14;

	}
	///End of Regions loading

	///
	//-------------------------------LOADING SECTION_DATA----------------------------------------------------
	///
	
	unsigned int section_count = *(unsigned int*)((char*)mode_header + 0x10 + 0x30);
	dfbt* section_header = Get_dfbt_from_size(my_dfbt_list, 0x68, mem_start);//need to find out the compression flag

	char* section_header_mem = (char*)(section_header)+0x10;
	bool uncompressed = !*(bool*)(section_header_mem + 0x1A);
	//each section_contains only one section_data BLOCK
	while (section_count--)
	{
		//time to look into the section_data
		dfbt* section_data_header = Get_dfbt_from_size(my_dfbt_list, 0xB4, mem_start);
		//Load the section_data and its child blocks
		section_data_list.emplace_back(section_data_header, my_dfbt_list,uncompressed);
		//update mem_start
		mem_start = (char*)section_data_header;
	}	
	///---------------------------------END OF SECTION LOADING

	///
	//----------------------------------NODES LOADING--------------------------------
	///
	unsigned int nodes_count = *(unsigned int*)((char*)mode_header + 0x10 + 0x60);

	if (nodes_count)
	{
		dfbt* nodes_header = Get_dfbt_from_size(my_dfbt_list, 0x60, mem_start);
		char* nodes_header_mem = (char*)nodes_header + 0x10;

		char* string_ptr = nodes_header_mem + 0x60 * nodes_count;

		while (nodes_count--)
		{
			__int8 nodes_name_length = *(__int8*)(nodes_header_mem + 0x3);

			nodes temp_nodes;

			temp_nodes.name = Get_string(string_ptr, nodes_name_length);

			temp_nodes.parentNode = *(__int16*)(nodes_header_mem + 0x4);
			temp_nodes.firstChildNode = *(__int16*)(nodes_header_mem + 0x6);
			temp_nodes.nextSiblingNode = *(__int16*)(nodes_header_mem + 0x8);
			temp_nodes.importNodeIndex = *(__int16*)(nodes_header_mem + 0xA);
			temp_nodes.defaultTranslation = *(vector3d*)(nodes_header_mem + 0xC);
			temp_nodes.defaultRotation = *(quaternion*)(nodes_header_mem + 0x18);
			temp_nodes.inverseScale = *(float*)(nodes_header_mem + 0x28);
			/*
			*(vector3d*)(nodes_header_mem + 0x2C) = { nodes_list[i].inverseForward.x,nodes_list[i].inverseLeft.x,nodes_list[i].inverseUp.x };
			*(vector3d*)(nodes_header_mem + 0x38) = { nodes_list[i].inverseForward.y,nodes_list[i].inverseLeft.y,nodes_list[i].inverseUp.y };
			*(vector3d*)(nodes_header_mem + 0x44) = { nodes_list[i].inverseForward.z,nodes_list[i].inverseLeft.z,nodes_list[i].inverseUp.z };
			*/
			//reading from column major into row major matrix
			temp_nodes.inverseForward = { *(float*)(nodes_header_mem + 0x2C),*(float*)(nodes_header_mem + 0x38),*(float*)(nodes_header_mem + 0x44) };
			temp_nodes.inverseLeft = { *(float*)(nodes_header_mem + 0x2C + 0x4),*(float*)(nodes_header_mem + 0x38 + 0x4),*(float*)(nodes_header_mem + 0x44 + 0x4) };
			temp_nodes.inverseUp = { *(float*)(nodes_header_mem + 0x2C + 0x8),*(float*)(nodes_header_mem + 0x38 + 0x8),*(float*)(nodes_header_mem + 0x44 + 0x8) };
			temp_nodes.inversePosition = *(vector3d*)(nodes_header_mem + 0x50);
			temp_nodes.distanceFromParent = *(float*)(nodes_header_mem + 0x5C);

			nodes_list.push_back(temp_nodes);

			string_ptr += nodes_name_length;
			nodes_header_mem += 0x60;
		}

		mem_start = (char*)nodes_header;
	}
	///---------------------END OF NODES LOADING

	///
	//-----------------------------------MARKERS LOADING----------------------------------------
	///
	unsigned int marker_group_count = *(unsigned int*)((char*)mode_header + 0x10 + 0x78);

	if (marker_group_count)
	{
		dfbt* marker_group_header = Get_dfbt_from_size(my_dfbt_list, 0x10, mem_start);
		char* marker_group_header_mem = (char*)marker_group_header + 0x10;

		mem_start = (char*)marker_group_header;

		while (marker_group_count--)
		{
			__int8 marker_group_name_length = *(__int8*)(marker_group_header_mem + 0x3);
			unsigned int markers_count = *(unsigned int*)(marker_group_header_mem + 0x4);

			//------At leaast one marker is required per marker_group
			if (markers_count)
			{
				markers_group temp_markers_group;

				dfbt* markers_header = Get_dfbt_from_size(my_dfbt_list, 0x24, mem_start);
				temp_markers_group.name = Get_string((char*)markers_header - marker_group_name_length, marker_group_name_length);

				char* marker_mem = (char*)markers_header + 0x10;

				while (markers_count--)
				{
					markers temp_markers;

					temp_markers.regionIndex = *(__int8*)(marker_mem);
					temp_markers.permutationIndex = *(__int8*)(marker_mem + 0x1);
					temp_markers.nodeIndex = *(__int8*)(marker_mem + 0x2);
					temp_markers.translation = *(vector3d*)(marker_mem + 0x4);
					temp_markers.rotation = *(quaternion*)(marker_mem + 0x10);
					temp_markers.scale = *(float*)(marker_mem + 0x20);

					temp_markers_group.markers_list.push_back(temp_markers);

					marker_mem += 0x24;
				}

				markers_groups_list.push_back(temp_markers_group);
				marker_group_header_mem += 0x10;
				mem_start = (char*)markers_header;
			}
		}
		
	}
	///------------------END of MArkers loading

	///
	//----------------------LOADING MATERIALS
	///
	unsigned int material_count = *(unsigned int*)((char*)mode_header + 0x10 + 0x84);

	if (material_count)
	{
		dfbt* material_header = Get_dfbt_from_size(my_dfbt_list, 0x34, mem_start);
		char* material_header_mem = (char*)material_header + 0x10;

		char* string_ptr = material_header_mem + material_count * 0x34;

		//we are basically concerned with the new materials
		while (material_count--)
		{
			material temp_material;

			unsigned int tag_loc_string_length = *(unsigned int*)(material_header_mem + 0x18);
			temp_material.shader = Get_string(string_ptr, tag_loc_string_length);

			material_list.push_back(temp_material);

			material_header_mem += 0x34;
			string_ptr += tag_loc_string_length + 1;
		}
		mem_start = (char*)material_header;
	}

	///i am too lazy to write a dfbt_list deletion logic :P
}
mode::~mode()
{
	delete[] mode_data;
}
void mode::Dump_obj_data(section_data& section_dat, string file)
{
	ofstream fout;
	fout.open(file, ios::out);

	//write vertices pos
	for (int i = 0; i < section_dat.vertex_list.size(); i++)
	{
		float x = section_dat.vertex_list[i].pos.x * 100;
		float y = section_dat.vertex_list[i].pos.y * 100;
		float z = section_dat.vertex_list[i].pos.z * 100;
		fout << "v  " << to_string(x) << ' ' << to_string(y) << ' ' << to_string(z) << '\n';
	}
	fout << "\n";//leave a line
	//write texture coordinates
	for (int i = 0; i < section_dat.vertex_list.size(); i++)
	{
		float x = section_dat.vertex_list[i].tex_cord.x;
		float y = section_dat.vertex_list[i].tex_cord.y;
		float z = 0.0f;
		fout << "vt  " << to_string(x) << ' ' << to_string(y) << ' ' << to_string(z) << '\n';
	}
	fout << "\n";//leave a line
	//write vertex normals
	for (int i = 0; i < section_dat.vertex_list.size(); i++)
	{
		float x = section_dat.vertex_list[i].normal.x;
		float y = section_dat.vertex_list[i].normal.y;
		float z = section_dat.vertex_list[i].normal.z;
		fout << "vn  " << to_string(x) << ' ' << to_string(y) << ' ' << to_string(z) << '\n';
	}
	fout << "\n";//leave a line
	
	//time to write the faces
	for (int k = 0; k < section_dat.parts_list.size(); k++)
	{
		parts temp_part = section_dat.parts_list[k];

		fout << "g group_" << to_string(k + 1) << '\n';

		for (int i = 0; i < temp_part.face_list.size(); i++)
		{
			triangle_face temp = temp_part.face_list[i];

			string tempy = to_string(temp.v0 + 1);
			fout << "f " << tempy << '/' << tempy << '/' << tempy << '\t';
			tempy = to_string(temp.v1 + 1);
			fout << tempy << '/' << tempy << '/' << tempy << '\t';
			tempy = to_string(temp.v2 + 1);
			fout << tempy << '/' << tempy << '/' << tempy << '\n';
		}
		cout << '\n';
	}
	//write the triangle strip data
	for (int k = 0; k < section_dat.parts_list.size(); k++)
	{
		int start_index = section_dat.parts_list[k].start_index;
		int length = section_dat.parts_list[k].length;

		fout << '#';
		for (int i = 0; i < length; i++)
			fout << to_string(section_dat.strip_index_list[start_index + i].vertex_index + 1) << '\t';
		fout << '\n';
	}

	fout.close();
}
//Dumps the render model into a collada file format
void mode::Dump_collada_data(string file)
{
	Assimp::Exporter exporter;
	const aiExportFormatDesc* format = exporter.GetExportFormatDescription(0);

	aiScene *test_scene=new aiScene();

	test_scene->mRootNode = new aiNode();
	test_scene->mRootNode->mName = "root";
	aiIdentityMatrix4(&test_scene->mRootNode->mTransformation);

	///we will no add a mesh to convert nodes to bone
	test_scene->mNumMeshes = 1;
	test_scene->mMeshes = new aiMesh*[1];
	test_scene->mMeshes[0] = new aiMesh();

	if (nodes_list.size())
		nodes_data_dump(0, test_scene->mRootNode, test_scene->mMeshes[0]);

	aiReturn ret = exporter.Export(test_scene, format->id, file, test_scene->mFlags);

}
//Dumps the render_model node data into the assimp stuff
///recursive implementation
void mode::nodes_data_dump(int current_node_index, aiNode* parent_node,aiMesh* global_mesh)
{
	//node adding codes
	aiNode** t_nodes = new aiNode*[parent_node->mNumChildren + 1];
	///store all the parent's previous children
	for (int i = 0; i < parent_node->mNumChildren; i++)
	{
		t_nodes[i] = parent_node->mChildren[i];
	}

	aiNode* t_node= new aiNode();
	t_node->mName = nodes_list[current_node_index].name;
	t_node->mParent = parent_node;	
	nodes t_current_node = nodes_list[current_node_index];
	aiMatrix4x4 local_node_transform({ 1,1,1 },
	{ t_current_node.defaultRotation.w,t_current_node.defaultRotation.i,t_current_node.defaultRotation.j,t_current_node.defaultRotation.k },
	{ t_current_node.defaultTranslation.x,t_current_node.defaultTranslation.y,t_current_node.defaultTranslation.z });
	t_node->mTransformation = local_node_transform;//actually its the transformation

	t_nodes[parent_node->mNumChildren] = t_node;
	delete [] parent_node->mChildren;///delete the older array
	parent_node->mChildren = t_nodes;	///assign to newer array
	parent_node->mNumChildren += 1;

	//node to bone conversion codes
	aiBone** t_bones = new aiBone*[global_mesh->mNumBones + 1];
	///store all the previous bone declarations
	for (int i = 0; i < global_mesh->mNumBones; i++)
	{
		t_bones[i] = global_mesh->mBones[i];
	}

	aiBone* t_current_bone = new aiBone();
	t_current_bone->mName = nodes_list[current_node_index].name;

	t_bones[global_mesh->mNumBones] = t_current_bone;
	delete[] global_mesh->mBones;
	global_mesh->mBones = t_bones;
	global_mesh->mNumBones += 1;
	
	if (nodes_list[current_node_index].nextSiblingNode != -1)
		nodes_data_dump(nodes_list[current_node_index].nextSiblingNode, parent_node, global_mesh);
	if (nodes_list[current_node_index].firstChildNode != -1)
		nodes_data_dump(nodes_list[current_node_index].firstChildNode, t_node, global_mesh);
		
}
//Dumps the loaded model data into a render_model file
void mode::Dump_render_model(string file_loc)
{
	ofstream fout(file_loc.c_str(), ios::binary | ios::out);

	if (fout.is_open())
	{
		//write the junk of 0x40 bytes
		int length = sizeof(_mode_base) / sizeof(_mode_base[0]);
		fout.write((char*)_mode_base, length);

		//creating a dfbt header
		dfbt _dfbt;
		_dfbt.padding = 0x74626664;

		//fixing dfbt header for base
		_dfbt.block_count = 0x1;
		_dfbt.block_size = 0xB8;

		fout.write((char*)&_dfbt, sizeof(dfbt));//the header

		//the tag base
		char tag_base[0xB8] = { 0 };
		//modifing the parameters
		*(__int8*)&tag_base[0x3] = name.length();
		*(__int16*)&tag_base[0x4] = 0x4;//force node maps flag
		*(int*)&tag_base[0x24] = region_list.size();
		*(int*)&tag_base[0x30] = section_data_list.size();
		*(int*)&tag_base[0x60] = nodes_list.size();
		*(int*)&tag_base[0x78] = markers_groups_list.size();
		*(int*)&tag_base[0x84] = material_list.size();

		fout.write(tag_base, 0xB8);			
		//write the model name
		fout.write(name.c_str(), name.length());

		if (region_list.size())
		{
			//creating regions header
			_dfbt.block_count = region_list.size();
			_dfbt.block_size = 0x14;

			fout.write((char*)&_dfbt, sizeof(dfbt));
		}

		//writing regions block
		for (int i = 0; i < region_list.size(); i++)
		{
			char regions[0x14] = { 0 };
			regions[0x3] = region_list[i].name.length();
			*(__int16*)&regions[0x4] = -1;//old node map offset
			*(int*)&regions[0x8] = region_list[i].perms.size();
			fout.write(regions, 0x14);
		}
		//writing regions name and permutations
		for (int i = 0; i < region_list.size(); i++)
		{
			fout.write(region_list[i].name.c_str(), region_list[i].name.length());
			//write the perms header
			if (region_list[i].perms.size())
			{
				_dfbt.block_count = region_list[i].perms.size();
				_dfbt.block_size = 0x10;

				fout.write((char*)&_dfbt, 0x10);
			}

			//write the perms block
			for (int j = 0; j < region_list[i].perms.size(); j++)
			{
				char perms[0x10] = { 0 };
				perms[0x3] = region_list[i].perms[j].name.length();
				*(__int16*)&perms[0x4] = region_list[i].perms[j].L1;
				*(__int16*)&perms[0x6] = region_list[i].perms[j].L2;
				*(__int16*)&perms[0x8] = region_list[i].perms[j].L3;
				*(__int16*)&perms[0xA] = region_list[i].perms[j].L4;
				*(__int16*)&perms[0xC] = region_list[i].perms[j].L5;
				*(__int16*)&perms[0xE] = region_list[i].perms[j].L6;

				fout.write(perms, 0x10);
			}
			//write the perms names
			for (int j = 0; j < region_list[i].perms.size(); j++)
				fout.write(region_list[i].perms[j].name.c_str(), region_list[i].perms[j].name.length());
		}
		if (section_data_list.size())
		{
			//creating the section header
			_dfbt.block_count = section_data_list.size();
			_dfbt.block_size = 0x68;

			fout.write((char*)&_dfbt, sizeof(dfbt));
		}

		//writing the section block
		for (int i = 0; i < section_data_list.size(); i++)
		{
			char section[0x68] = { 0 };
			*(__int16*)section = 2;//geo classification rigid boned
			*(__int16*)&section[0x18] = 2;//geo classification again
			*(int*)&section[0x34] = 1;
			fout.write((char*)section, 0x68);
		}
		//writing the section data,each section contains only one section DATA
		for (int i = 0; i < section_data_list.size(); i++)
		{
			fout.write((char*)FINS, 0x10);//starting of section child elements
			
			_dfbt.block_count = 0x1;
			_dfbt.block_size = 0xB4;
			//section data header
			fout.write((char*)&_dfbt, sizeof(dfbt));
			//section data
			char section_data[0xB4] = { 0 };
			*(int*)&section_data[0x0] = section_data_list[i].parts_list.size();
			*(int*)&section_data[0x24] = section_data_list[i].vertex_list.size();

			//calculating stip indexes length
			int strip_indexes_length=0;
			for (int j = 0; j < section_data_list[i].parts_list.size(); j++)
				strip_indexes_length += section_data_list[i].parts_list[j].face_list.size() * 3;

			*(int*)&section_data[0x30] = strip_indexes_length;
			*(int*)&section_data[0xA4] = section_data_list[i].node_map_list.size();
			fout.write(section_data, 0xB4);

			fout.write((char*)TCES, 0x10);//starting of section data child BLOCKs section(global_geometry_section_struct_block)

			//---------------------------well i guess u cannot render anything without parts
			//parts header
			_dfbt.block_count = section_data_list[i].parts_list.size();
			_dfbt.block_size = 0x48;

			fout.write((char*)&_dfbt, sizeof(dfbt));

			//Strip indexes list
			list<int> strip_index_list;

			//write the parts block
			for (int j = 0; j < section_data_list[i].parts_list.size(); j++)
			{
				char parts[0x48] = { 0 };
				list<int> temp_list = Generate_strip_indices(section_data_list[i].parts_list[j]);

				*(__int16*)&parts[0] = 2;
				*(__int16*)&parts[2] = 0x1 | 0x2 | 0x8;
				*(__int16*)&parts[4] = section_data_list[i].parts_list[j].material_index;
				*(__int16*)&parts[6] = strip_index_list.size();
				*(__int16*)&parts[8] = temp_list.size();
				*(vector3d*)&parts[0x10] = Calculate_centroid(section_data_list[i].parts_list[j], section_data_list[i]);

				strip_index_list.insert(strip_index_list.cend(), temp_list.cbegin(), temp_list.cend());

				fout.write(parts, 0x48);
			}
			//---------------------------what is a model file without any vertex
			//RAW vertices header
			_dfbt.block_count = section_data_list[i].vertex_list.size();
			_dfbt.block_size = 0xC4;

			fout.write((char*)&_dfbt, sizeof(dfbt));

			//write the RAW vertex block
			for (int j = 0; j < section_data_list[i].vertex_list.size(); j++)
			{
				char vertex[0xC4] = { 0 };

				RAW_vertex &temp = section_data_list[i].vertex_list[j];

				*(vector3d*)(vertex) = temp.pos;

				for (int i = 0; i < 4; i++)
				{
					*(int*)(vertex + 0x2C + i * 0x4) = temp.nodes[i].index;
					*(float*)(vertex + 0x1C + i * 0x4) = temp.nodes[i].weight;			
					
				}

				*(int*)(vertex + 0x3C) = 0x1;

				*(float*)(vertex + 0x44) = temp.tex_cord.x;
				*(float*)(vertex + 0x48) = 1 - temp.tex_cord.y;			//<--------some UV crap on its way
						
				*(vector3d*)(vertex + 0x4C) = temp.normal;
				*(vector3d*)(vertex + 0x58) = temp.binormal;
				*(vector3d*)(vertex + 0x64) = temp.tangent;
				*(vector3d*)(vertex + 0x70) = temp.anisotropic_binormal;


				fout.write(vertex, 0xC4);
			}
			if (strip_index_list.size())
			{
				//strip index header
				_dfbt.block_count = strip_index_list.size();
				_dfbt.block_size = 0x2;

				fout.write((char*)&_dfbt, sizeof(dfbt));
			}

			//write the strip index block
			while(strip_index_list.size())
			{
				char strip_index[2] = { 0 };
				*(__int16*)&strip_index[0] = (__int16)*strip_index_list.cbegin();
				strip_index_list.pop_front();
				fout.write(strip_index, 0x2);
			}

			fout.write((char*)TADP, 0x10);//end of section data child BLOCKs section(global_geometry_section_struct_block)

			if (section_data_list[i].node_map_list.size())
			{
				//node map header
				_dfbt.block_count = section_data_list[i].node_map_list.size();
				_dfbt.block_size = 0x1;

				fout.write((char*)&_dfbt, sizeof(dfbt));
			}

			//write the node map header block
			for (int j = 0; j < section_data_list[i].node_map_list.size(); j++)
				fout.write(&section_data_list[i].node_map_list[j].node_index, 0x1);	

			fout.write((char*)KOLB, 0x10);//end of section child elements
		}
		
		if (nodes_list.size())
		{
			//nodes header
			_dfbt.block_count = nodes_list.size();
			_dfbt.block_size = 0x60;

			fout.write((char*)&_dfbt, sizeof(dfbt));
		}

		//nodes writing stuff
		for (int i = 0; i < nodes_list.size(); i++)
		{
			char nodes_header_mem[0x60] = { 0 };

			*(__int8*)(nodes_header_mem + 0x3) = nodes_list[i].name.length();
			*(__int16*)(nodes_header_mem + 0x4) = nodes_list[i].parentNode;
			*(__int16*)(nodes_header_mem + 0x6) = nodes_list[i].firstChildNode;
			*(__int16*)(nodes_header_mem + 0x8) = nodes_list[i].nextSiblingNode;
			*(__int16*)(nodes_header_mem + 0xA) = nodes_list[i].importNodeIndex;
			*(vector3d*)(nodes_header_mem + 0xC) = nodes_list[i].defaultTranslation;
			*(quaternion*)(nodes_header_mem + 0x18) = nodes_list[i].defaultRotation;
			*(float*)(nodes_header_mem + 0x28) = nodes_list[i].inverseScale;
			//writing row major matrix into column major format
			*(vector3d*)(nodes_header_mem + 0x2C) = { nodes_list[i].inverseForward.x,nodes_list[i].inverseLeft.x,nodes_list[i].inverseUp.x };
			*(vector3d*)(nodes_header_mem + 0x38) = { nodes_list[i].inverseForward.y,nodes_list[i].inverseLeft.y,nodes_list[i].inverseUp.y };
			*(vector3d*)(nodes_header_mem + 0x44) = { nodes_list[i].inverseForward.z,nodes_list[i].inverseLeft.z,nodes_list[i].inverseUp.z };
			*(vector3d*)(nodes_header_mem + 0x50) = nodes_list[i].inversePosition;
			*(float*)(nodes_header_mem + 0x5C) = nodes_list[i].distanceFromParent;

			fout.write(nodes_header_mem, 0x60);
		}
		//for writing the names of nodes
		for (int i = 0; i < nodes_list.size(); i++)
		{
			fout.write(nodes_list[i].name.c_str(), nodes_list[i].name.length());
		}
		if (markers_groups_list.size())
		{
			//marker group header
			_dfbt.block_count = markers_groups_list.size();
			_dfbt.block_size = 0x10;

			fout.write((char*)&_dfbt, sizeof(dfbt));
		}
		//marker group writing stuff
		for (int i = 0; i < markers_groups_list.size(); i++)
		{
			char mark_g_dat[0x10] = { 0 };

			*(__int8*)(mark_g_dat + 0x3) = markers_groups_list[i].name.length();
			*(unsigned int*)(mark_g_dat + 0x4) = markers_groups_list[i].markers_list.size();

			fout.write(mark_g_dat, 0x10);
		}
		//for writing marker stuff
		for (int i = 0; i < markers_groups_list.size(); i++)
		{
			//marker group name
			fout.write(markers_groups_list[i].name.c_str(), markers_groups_list[i].name.length());

			if (markers_groups_list[i].markers_list.size())
			{
				//marker header
				_dfbt.block_count = markers_groups_list[i].markers_list.size();
				_dfbt.block_size = 0x24;

				fout.write((char*)&_dfbt, sizeof(dfbt));
			}
			//marker data
			for (int j = 0; j < markers_groups_list[i].markers_list.size(); j++)
			{
				char marker_dat[0x24] = { 0 };

				*(__int8*)(marker_dat) = markers_groups_list[i].markers_list[j].regionIndex;
				*(__int8*)(marker_dat + 0x1) = markers_groups_list[i].markers_list[j].permutationIndex;
				*(__int8*)(marker_dat + 0x2) = markers_groups_list[i].markers_list[j].nodeIndex;
				*(vector3d*)(marker_dat + 0x4) = markers_groups_list[i].markers_list[j].translation;
				*(quaternion*)(marker_dat + 0x10) = markers_groups_list[i].markers_list[j].rotation;
				*(float*)(marker_dat + 0x20) = markers_groups_list[i].markers_list[j].scale;

				fout.write(marker_dat, 0x24);
			}
		}
		if (material_list.size())
		{
			//materials header
			_dfbt.block_count = material_list.size();
			_dfbt.block_size = 0x34;

			fout.write((char*)&_dfbt, sizeof(dfbt));
		}

		//materials data
		for (int i = 0; i < material_list.size(); i++)
		{
			char mat_dat[] = { 0x64,0x61,0x68,0x73,0x40,0x11,0x40,0x08,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x64,0x61,0x68,0x73,0xD0,0x18,0x40,0x08,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x44,0x4D,0x95,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

			*(unsigned int*)(mat_dat + 0x18) = material_list[i].shader.length();

			fout.write(mat_dat, 0x34);
		}
		for (int i = 0; i < material_list.size(); i++)
		{
			fout.write(material_list[i].shader.c_str(), material_list[i].shader.length() + 1);
		}		
	}

	fout.close();
}
//Calculates the centroid of the specified part
vector3d mode::Calculate_centroid(parts& my_parts,section_data& my_section)
{
	vector3d ret;
	ret.x = ret.y = ret.z = 0;

	for (int i = 0; i < my_parts.face_list.size(); i++)
	{
		RAW_vertex v0 = my_section.vertex_list[my_parts.face_list[i].v0];
		RAW_vertex v1 = my_section.vertex_list[my_parts.face_list[i].v1];
		RAW_vertex v2 = my_section.vertex_list[my_parts.face_list[i].v2];

		ret.x += (v0.pos.x + v1.pos.x + v2.pos.x) / 3;
		ret.y += (v0.pos.y + v1.pos.y + v2.pos.y) / 3;
		ret.z += (v0.pos.z + v1.pos.z + v2.pos.z) / 3;
	}
	ret.x /= my_parts.face_list.size();
	ret.y /= my_parts.face_list.size();
	ret.z /= my_parts.face_list.size();

	return ret;
}
//Generates a strip index list from parts
list<int> mode::Generate_strip_indices(parts& my_parts)
{
	list<int> ret;

	for (int i = 0; i < my_parts.face_list.size(); i++)
	{
		ret.push_back(my_parts.face_list[i].v0);
		ret.push_back(my_parts.face_list[i].v1);
		ret.push_back(my_parts.face_list[i].v2);
	}

	return ret;
}
//normalises the node weigths in the parts
void __vertex_normalise_weigths(RAW_vertex* p)
{
	float magnitude = sqrt(pow(p->nodes[0].weight, 2) + pow(p->nodes[1].weight, 2) + pow(p->nodes[2].weight, 2) + pow(p->nodes[3].weight, 2));
	p->nodes[0].weight /= magnitude;
	p->nodes[1].weight /= magnitude;
	p->nodes[2].weight /= magnitude;
	p->nodes[3].weight /= magnitude;
}

#pragma region model_import
section_data::section_data(const aiScene* my_scene, int material_start_index,const vector<nodes> &nodes_list)
{
	this->material_start_index = material_start_index;

	aiNode* root_node = my_scene->mRootNode;

	for (int i = 0; i < root_node->mNumChildren; i++)
		Load_mesh_recursive(root_node->mChildren[i], my_scene,nodes_list);

	//normalize the node weights
	for (int i = 0; i < vertex_list.size(); i++)
		__vertex_normalise_weigths(&vertex_list[i]);
}
void section_data::Load_mesh_recursive(aiNode* node, const aiScene* my_scene, const vector<nodes> &nodes_list)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		int vertex_start_index = vertex_list.size();

		int mesh_index = node->mMeshes[i];
		aiMesh* t_mesh = my_scene->mMeshes[mesh_index];

		for (int j = 0; j < t_mesh->mNumVertices; j++)
		{
			RAW_vertex t_RAWvertex = { 0 };

			aiVector3D t_vertex_pos = t_mesh->mVertices[j];

			aiNode* t_parent = node;
			while (t_parent!=my_scene->mRootNode)
			{
				const aiMatrix4x4 t_mat = t_parent->mTransformation;
				
				aiTransformVecByMatrix4(&t_vertex_pos, &t_mat);
				t_parent = t_parent->mParent;
			}

			t_RAWvertex.pos.x = t_vertex_pos.x / 100;
			t_RAWvertex.pos.y = t_vertex_pos.y / 100;
			t_RAWvertex.pos.z = t_vertex_pos.z / 100;

			if (t_mesh->HasTangentsAndBitangents())
			{
				aiVector3D t_vertex_binorm = t_mesh->mBitangents[j];
				aiVector3D t_vertex_tan = t_mesh->mTangents[j];

				t_RAWvertex.binormal.x = t_vertex_binorm.x;
				t_RAWvertex.binormal.y = t_vertex_binorm.y;
				t_RAWvertex.binormal.z = t_vertex_binorm.z;
				t_RAWvertex.tangent.x = t_vertex_tan.x;
				t_RAWvertex.tangent.y = t_vertex_tan.y;
				t_RAWvertex.tangent.z = t_vertex_tan.z;
			}
			if (t_mesh->HasNormals())
			{
				aiVector3D t_vertex_norm = t_mesh->mNormals[j];

				t_RAWvertex.normal.x = t_vertex_norm.x;
				t_RAWvertex.normal.y = t_vertex_norm.y;
				t_RAWvertex.normal.z = t_vertex_norm.z;
			}
			if (t_mesh->HasTextureCoords(0))
			{
				aiVector3D t_vertex_uv = t_mesh->mTextureCoords[0][j];

				t_RAWvertex.tex_cord.x = t_vertex_uv.x;
				t_RAWvertex.tex_cord.y = t_vertex_uv.y;
			}

			vertex_list.push_back(t_RAWvertex);
		}
		//now add the node weights
		//render_model supports upto 4 nodes only
		for (int j = 0; j < t_mesh->mNumBones; j++)
		{
			aiBone* t_bone = t_mesh->mBones[j];
			aiVertexWeight* t_weight = t_bone->mWeights;

			int t_node_map_index = _get_node_map_by_node_name(t_bone->mName.C_Str(), nodes_list);

			for (int k = 0; k <  t_bone->mNumWeights; k++)
			{
				if (t_weight[k].mWeight > vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[0].weight)
				{
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[0].index = t_node_map_index;
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[0].weight = t_weight[k].mWeight;
				}
				else if (t_weight[k].mWeight>vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[1].weight)
				{
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[1].index = t_node_map_index;
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[1].weight = t_weight[k].mWeight;
				}
				else if (t_weight[k].mWeight > vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[2].weight)
				{
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[2].index = t_node_map_index;
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[2].weight = t_weight[k].mWeight;
				}
				else if (t_weight[k].mWeight > vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[3].weight)
				{
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[3].index = t_node_map_index;
					vertex_list[vertex_start_index + t_weight[k].mVertexId].nodes[3].weight = t_weight[k].mWeight;
				}
			}
		}

		parts t_part = { 0 };
		//there is a mesh per material in Assimp
		t_part.material_index = material_start_index + (short)t_mesh->mMaterialIndex;

		aiFace* t_face = t_mesh->mFaces;
		for (int j = 0; j < t_mesh->mNumFaces; j++)
		{
			//supposed to handle only three faces comprising of three vertices
			unsigned int* t_indices = t_face[j].mIndices;
			triangle_face t_face = { 0 };
			t_face.v0 = vertex_start_index + t_indices[0];
			t_face.v1 = vertex_start_index + t_indices[1];
			t_face.v2 = vertex_start_index + t_indices[2];

			if (t_face.v0 < vertex_list.size() && t_face.v1 < vertex_list.size() && t_face.v2 < vertex_list.size())
				t_part.face_list.push_back(t_face);
			else
				cout << "\nStray vertex found";
		}
		parts_list.push_back(t_part);

	}

	for (int i = 0; i < node->mNumChildren; i++)
		Load_mesh_recursive(node->mChildren[i], my_scene,nodes_list);

}
int section_data::_get_node_map_by_node_name(std::string bone_name, const vector<nodes> &nodes_list)
{

	for (int i = 0; i < node_map_list.size(); i++)
	{
		int node_index = node_map_list[i].node_index;

		if (bone_name.compare(nodes_list[node_index].name) == 0)
			return i;
	}
	node_map t_Nmap;
	for (int i = 0; i < nodes_list.size(); i++)
	{
		if (bone_name.compare(nodes_list[i].name) == 0)
		{
			t_Nmap.node_index = i;
			break;
		}
	}
	node_map_list.push_back(t_Nmap);

	return node_map_list.size() - 1;
}
mode::mode(render_model_import::render_model_import& my_import)
{
	name = my_import.model_name;
	int section_index = 0;
	int material_start_index = 0;

	//adding node data
	if (my_import.armature_file.length())
	{
		const aiScene* armature = aiImportFile(my_import.armature_file.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
		for (int i = 0; i < armature->mNumMeshes; i++)
		{
			for (int j = 0; j < armature->mMeshes[i]->mNumBones; j++)
				Load_bone(armature->mMeshes[i]->mBones[j]->mName.C_Str(), armature);
		}
		Link_bones(armature);
		aiReleaseImport(armature);
	}
	//will be adding marker data shortly

	for (int i = 0; i < my_import.region_list.size(); i++)
	{
		regions t_regions;
		t_regions.name = my_import.region_list[i].name;		

		for (int j = 0; j < my_import.region_list[i].perms_list.size(); j++)
		{
			permutations t_perm;
			t_perm.name = my_import.region_list[i].perms_list[j].name;

			for (int k = 0; k < 6; k++)//there are 6 LOD
			{
				int t_size = my_import.region_list[i].perms_list[j].model_files.size();
				if (k < t_size)
				{					
					const aiScene* import_model = aiImportFile(my_import.region_list[i].perms_list[j].model_files[k].c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
					if (import_model == NULL) 
					{
						cout << "\n---------IMPORT ERROR------------ \n" << my_import.region_list[i].perms_list[j].model_files[k].c_str() << '\n' << aiGetErrorString();
						getch();//halt
						exit(0x0);
					}
					section_data t_section(import_model, material_start_index, nodes_list);
					section_data_list.push_back(t_section);

					//all should have same material libraries(same materials) same as the LOD1
					if (k == 0)
					{
						for (int l = 0; l < import_model->mNumMaterials; l++)
						{
							material t_mat;
							t_mat.shader = my_import.region_list[i].perms_list[j].name + "_material_" + to_string(l);
							material_list.push_back(t_mat);
						}
						material_start_index += import_model->mNumMaterials;
					}
					((__int8*)&t_perm.L1)[k] = section_index++;
					aiReleaseImport(import_model);
				}
				else
				{
					((__int8*)&t_perm.L1)[k] = section_index - 1;//using the previous section to fill empty LODs
				}
			}
			t_regions.perms.push_back(t_perm);
		}
		region_list.push_back(t_regions);
	}
}
//returns the index of node by the given name
void mode::Load_bone(std::string name, const aiScene* my_scene)
{
	aiNode* current_node = my_scene->mRootNode->FindNode(name.c_str());

	nodes t_node = { "",0 };
	t_node.name = current_node->mName.C_Str();
	t_node.parentNode = -1;
	t_node.firstChildNode = -1;
	t_node.nextSiblingNode = -1;
	t_node.importNodeIndex = nodes_list.size();

	aiMatrix4x4 t_mat = current_node->mTransformation;

	aiVector3D t_ainode_pos;
	aiQuaternion t_ainode_rot;

	t_mat.DecomposeNoScaling(t_ainode_rot, t_ainode_pos);

	t_node.defaultTranslation.x = t_ainode_pos.x / 100;
	t_node.defaultTranslation.y = t_ainode_pos.y / 100;
	t_node.defaultTranslation.z = t_ainode_pos.z / 100;

	t_node.defaultRotation.i = t_ainode_rot.x;
	t_node.defaultRotation.j = t_ainode_rot.y;
	t_node.defaultRotation.k = t_ainode_rot.z;
	t_node.defaultRotation.w = t_ainode_rot.w;

	t_node.distanceFromParent = t_ainode_pos.Length() / 100;

	aiNode* t_parent = current_node->mParent;
	while (t_parent != my_scene->mRootNode)
	{
		//corrected multiplication operation to P*T
		aiMatrix4x4 P_mat = t_parent->mTransformation;
		aiMultiplyMatrix4(&P_mat, &t_mat);
		t_mat = P_mat;

		t_parent = t_parent->mParent;
	}
	t_mat.Inverse();	

	t_node.inverseScale = 1.0f;

	t_node.inverseForward = { t_mat.a1,t_mat.a2,t_mat.a3 };
	t_node.inverseLeft = { t_mat.b1,t_mat.b2,t_mat.b3 };
	t_node.inverseUp = { t_mat.c1,t_mat.c2,t_mat.c3 };
	t_node.inversePosition = { t_mat.a4 / 100,t_mat.b4 / 100,t_mat.c4 / 100 };

	nodes_list.push_back(t_node);
	
}
//reverted to the original logic
void mode::Link_bones(const aiScene* my_scene)
{
	for (int i = 0; i < nodes_list.size(); i++)
	{
		aiNode* t_ainode = my_scene->mRootNode->FindNode(nodes_list[i].name.c_str());

		//fixing parent index
		if (t_ainode->mParent != my_scene->mRootNode)
			nodes_list[i].parentNode = (short)find_node_in_node_list(t_ainode->mParent->mName.C_Str());
		
		//fixing child and next sibbiling indexes
		int prev_child_index;
		for (int j = 0; j < t_ainode->mNumChildren; j++)
		{
			int t = find_node_in_node_list(t_ainode->mChildren[j]->mName.C_Str());
			if (j)
			{
				//sibbilings
				nodes_list[prev_child_index].nextSiblingNode = (short)t;
			}
			else
			{
				//first child
				nodes_list[i].firstChildNode = (short)t;
			}
			prev_child_index = t;
		}
	}
}
int mode::find_node_in_node_list(std::string name)
{
	for (int i = 0; i < nodes_list.size(); i++)
	{
		if (nodes_list[i].name.compare(name) == 0)
			return i;
	}
	return -1;
}
//Each JMS file has its material declaration allowing materials of the same name to define different region,perm and LOD
//i.e. different sections declared under the material definitions
int mode::_get_section_index(const jms::material& mat_def)
{
	///look for exisitng identicals
	int i = 0;
	while(1)
	{
		if (i == region_list.size())
		{
			regions t_region;
			t_region.name = mat_def.Region;
			region_list.push_back(t_region);
		}

		regions& t_region_ref = region_list[i];
		if (mat_def.Region.compare(t_region_ref.name) == 0)
		{			
			int j = 0;
			while (1)
			{
				if (j == t_region_ref.perms.size())
				{
					permutations t_perm;
					t_perm.name = mat_def.Permutation;
					t_region_ref.perms.push_back(t_perm);
				}

				permutations& t_perm_ref = t_region_ref.perms[j];
				if (mat_def.Permutation.compare(t_perm_ref.name) == 0)
				{
					section_data t_section;
					int ret = section_data_list.size();

					switch (mat_def.LOD[1])
					{
					case '1':
						if (t_perm_ref.L1 != -1)
							ret = t_perm_ref.L1;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L1 = (short)ret;
						}
						break;
					case '2':
						if (t_perm_ref.L2 != -1)
							ret = t_perm_ref.L2;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L2 = (short)ret;
						}
						break;
					case '3':
						if (t_perm_ref.L3 != -1)
							ret = t_perm_ref.L3;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L3 = (short)ret;
						}
						break;
					case '4':
						if (t_perm_ref.L4 != -1)
							ret = t_perm_ref.L4;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L4 = (short)ret;
						}
						break;
					case '5':
						if (t_perm_ref.L5 != -1)
							ret = t_perm_ref.L5;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L5 = (short)ret;
						}
						break;
					case '6':
						if (t_perm_ref.L6 != -1)
							ret = t_perm_ref.L6;
						else
						{
							section_data_list.push_back(t_section);
							t_perm_ref.L6 = (short)ret;
						}
						break;
					}
					return ret;
				}
				j++;
			}			
		}
		i++;
	}

}
int mode::_get_material_index(const jms::material& mat_def)
{
	for (int i = 0; i < material_list.size(); i++)
		if (mat_def.name.compare(material_list[i].shader)==0)
			return i;

	material t_mat;
	t_mat.shader = mat_def.name;
	material_list.push_back(t_mat);
	
	return material_list.size() - 1;
}
mode::mode(render_model_import::jms_model_import& my_import)
{
	name = my_import.model_name;

	for (int jms_file_iter = 0; jms_file_iter < my_import.model_files.size(); jms_file_iter++)
	{
		jms::jms* my_file = new jms::jms(my_import.model_files[jms_file_iter]);

		if (jms_file_iter == 0)
			Load_bones(my_file);
		Load_markers(my_file);

		vector<JMS_section_intermediate> intermediate_list;
		//iterate through material declaration and generate intermediate structures to be added into respective sections
		for (int jms_mat_iter = 0; jms_mat_iter < my_file->material_list.size(); jms_mat_iter++)
		{
			jms::material& t_mat = my_file->material_list[jms_mat_iter];

			JMS_section_intermediate t_intermediate = JMS_section_intermediate(_get_section_index(t_mat), _get_material_index(t_mat), my_file->vertex_list.size());
		
			intermediate_list.push_back(t_intermediate);
		}
		//add triangles to respective intermediate sections
		for (int triangle_index = 0; triangle_index < my_file->triangle_list.size(); triangle_index++)
		{
			jms::triangle& t_triangle = my_file->triangle_list[triangle_index];

			intermediate_list[t_triangle.shader_index]._add_triangle(t_triangle, my_file->vertex_list);
		}		
		//now add intermediates to their sections in form of parta
		for (int intermediate_iter = 0; intermediate_iter < intermediate_list.size(); intermediate_iter++)
		{
			JMS_section_intermediate& current_intermediate = intermediate_list[intermediate_iter];
			section_data& current_section = section_data_list[current_intermediate.section_pallete_index];

			parts t_part;

			t_part.start_index = current_section.strip_index_list.size();
			t_part.length = current_intermediate.t_triangle_list.size() * 0x3;
			t_part.material_index = current_intermediate.material_pallete_index;

			int vertex_start_offset = current_section.vertex_list.size();

			///current_section.vertex_list.insert(current_section.vertex_list.end(), current_intermediate.t_vertex_list.begin(), current_intermediate.t_vertex_list.end());
			//add RAW vertices
			for (int i = 0; i < current_intermediate.t_vertex_list.size(); i++)
			{
				jms::vertex& t_vertex = current_intermediate.t_vertex_list[i];

				RAW_vertex t_RAW_vertex = { 0 };
				
				t_RAW_vertex.pos.x = t_vertex.position.x*0.01;
				t_RAW_vertex.pos.y = t_vertex.position.y*0.01;
				t_RAW_vertex.pos.z = t_vertex.position.z*0.01;

				t_RAW_vertex.normal.x = t_vertex.normal.x;
				t_RAW_vertex.normal.y = t_vertex.normal.y;
				t_RAW_vertex.normal.z = t_vertex.normal.z;

				t_RAW_vertex.tex_cord.x = t_vertex.tex_cords.x;
				t_RAW_vertex.tex_cord.y = t_vertex.tex_cords.y;

				///reset the node weights 
				for (int j = 0; j < 4; j++)
				{
					t_RAW_vertex.nodes[j].index = -1;
					t_RAW_vertex.nodes[j].weight = 0.0f;
				}
				
				for (int j = 0; j < t_vertex.node_indices.size(); j++)
				{
					//node map stuff
					t_RAW_vertex.nodes[j].index = current_section._get_node_map_by_node_name(my_file->node_list[t_vertex.node_indices[j]].name, nodes_list);
					t_RAW_vertex.nodes[j].weight = t_vertex.node_weights[j];
				}

				current_section.vertex_list.push_back(t_RAW_vertex);
			}
			//add triangle faces 
			for (int face_iter = 0; face_iter < current_intermediate.t_triangle_list.size(); face_iter++)
			{
				jms::triangle& t_triangle = current_intermediate.t_triangle_list[face_iter];

				triangle_face t_face;
				
				t_face.v0 = vertex_start_offset + t_triangle.vertex_indices[0];
				t_face.v1 = vertex_start_offset + t_triangle.vertex_indices[1];
				t_face.v2 = vertex_start_offset + t_triangle.vertex_indices[2];

				t_part.face_list.push_back(t_face);
			}
			current_section.parts_list.push_back(t_part);
		}
		delete my_file;
	}
	///clean redundant vertices
	///call tangent and binormal generating function
	for (int i = 0; i < section_data_list.size(); i++)
	{
		//section_data_list[i].Remove_redundant_vertices();
		section_data_list[i].Calculate_vertex_normal_tangent_and_binormal();
	}
}
void mode::Load_bones(jms::jms* my_file)
{
	//Rearranging JMS nodes in various layers
	///global base
	JMS_node_intermediate node_base = JMS_node_intermediate("", { 0,0,0 }, { 0,0,0,0 }, nullptr);

	for (int i = 0; i < my_file->node_list.size(); i++)
	{
		jms::node& t_JMS_node = my_file->node_list[i];

		JMS_node_intermediate* t_intJMS_parent;

		//here i assume that parent nodes would be mentioned before child nodes,at least
		if (t_JMS_node.parent_node_index != -1)
		{
			t_intJMS_parent = node_base._get_child_node_by_name(my_file->node_list[t_JMS_node.parent_node_index].name);
			if (t_intJMS_parent == nullptr)
				throw new exception("Incorrect sequence of nodes,Parent node should come first");
		}
		else
			t_intJMS_parent = &node_base;

		///terrible declaraion and initialisation,i know
		JMS_node_intermediate* t_intJMS_node = new JMS_node_intermediate(
			t_JMS_node.name,
			{ t_JMS_node.position.x*0.01f,t_JMS_node.position.y*0.01f,t_JMS_node.position.z*0.01f },
			{ t_JMS_node.rotation.x,t_JMS_node.rotation.y,t_JMS_node.rotation.z,t_JMS_node.rotation.w },
			t_intJMS_parent);

		t_intJMS_parent->_add_child_node(t_intJMS_node);
	}
	//Actual addition into node pallete
	int layer = 0;
	vector<JMS_node_intermediate*> layer_nodes;

	int import_node_index = 0;
	
	node_base._get_child_node_by_layer(layer_nodes, layer);
	while (layer_nodes.size() != 0)
	{
		//add all those nodes into node_pallete
		for (int i = 0; i < layer_nodes.size(); i++)
		{
			nodes t_node;

			t_node.name = layer_nodes[i]->name;
			t_node.parentNode = find_node_in_node_list(layer_nodes[i]->parent_node->name);
			t_node.firstChildNode = -1;
			t_node.nextSiblingNode = -1;
			t_node.importNodeIndex = import_node_index++;
			t_node.inverseScale = 1.0f;

			//node transform
			real_matrix4x3 node_transform = real_matrix4x3(layer_nodes[i]->rotation, layer_nodes[i]->position);

			if (t_node.parentNode != -1)
			{
				//if has parent then calculate its translation and rotation wrt to parent
				real_point3d local_position;
				real_quaternion local_rotation;

				real_matrix4x3 parent_transform = real_matrix4x3(layer_nodes[i]->parent_node->rotation, layer_nodes[i]->parent_node->position);
				parent_transform.inverse();
				
				real_matrix4x3 local_transform;
				real_matrix4x3::multiply(local_transform, parent_transform, node_transform );

				local_transform.decompose_matrix(local_position, local_rotation);

				//relative positioning
				t_node.defaultTranslation = { local_position.x,local_position.y,local_position.z };
				t_node.defaultRotation = { local_rotation.i,local_rotation.j,local_rotation.k ,local_rotation.w };
			}
			else
			{
				t_node.defaultTranslation = { layer_nodes[i]->position.x ,layer_nodes[i]->position.y ,layer_nodes[i]->position.z };
				t_node.defaultRotation = { layer_nodes[i]->rotation.i,layer_nodes[i]->rotation.j,layer_nodes[i]->rotation.k,layer_nodes[i]->rotation.w };
			}

			//now the node inverse
			node_transform.inverse();
			t_node.inverseForward = { node_transform.forward.i,node_transform.forward.j,node_transform.forward.k };
			t_node.inverseLeft = { node_transform.left.i,node_transform.left.j,node_transform.left.k };
			t_node.inverseUp = { node_transform.up.i,node_transform.up.j,node_transform.up.k };
			t_node.inversePosition = { node_transform.translation.x,node_transform.translation.y,node_transform.translation.z };

			if (t_node.parentNode != -1)
				t_node.distanceFromParent = sqrt(
					t_node.defaultTranslation.x*t_node.defaultTranslation.x
					+ t_node.defaultTranslation.y*t_node.defaultTranslation.y
					+ t_node.defaultTranslation.z*t_node.defaultTranslation.z);
			else
				t_node.distanceFromParent = 0.0f;

			//now look for the parent and iterate through siblings
			if (nodes_list.size())
			{
				if (nodes_list[t_node.parentNode].firstChildNode == -1)
					nodes_list[t_node.parentNode].firstChildNode = nodes_list.size();
				else
				{
					int t = nodes_list[t_node.parentNode].firstChildNode;
					while (1)
					{
						if (nodes_list[t].nextSiblingNode == -1)
						{
							nodes_list[t].nextSiblingNode = nodes_list.size();
							break;
						}
						t = nodes_list[t].nextSiblingNode;
					}
				}
			}

			nodes_list.push_back(t_node);
		}
		layer_nodes.clear();
		layer = layer + 1;
		node_base._get_child_node_by_layer(layer_nodes, layer);
	}
}
void mode::Load_markers(jms::jms* my_file)
{
	for (int i = 0; i < my_file->marker_list.size(); i++)
	{
		jms::marker& jms_marker = my_file->marker_list[i];
		
		int marker_group_index = _get_marker_group(jms_marker.name);

		markers t_marker = { 0 };

		t_marker.regionIndex = -1;
		t_marker.permutationIndex = -1;
		t_marker.nodeIndex = find_node_in_node_list(my_file->node_list[jms_marker.parent_node].name);
		t_marker.translation.x = jms_marker.position.x*0.01;
		t_marker.translation.y = jms_marker.position.y*0.01;
		t_marker.translation.z = jms_marker.position.z*0.01;
		t_marker.rotation.i = jms_marker.rotation.x;
		t_marker.rotation.j = jms_marker.rotation.y;
		t_marker.rotation.k = jms_marker.rotation.z;
		t_marker.rotation.w = jms_marker.rotation.w;

		markers_groups_list[marker_group_index].markers_list.push_back(t_marker);
	}
}
int mode::_get_marker_group(std::string name)
{
	int ret = markers_groups_list.size();

	for (int i = 0; i < markers_groups_list.size(); i++)
	{
		if (markers_groups_list[i].name.compare(name) == 0)
			return i;
	}
	//not found create and add a new group
	markers_group t_mgroup;
	t_mgroup.name = name;
	t_mgroup.markers_list.resize(0);
	markers_groups_list.push_back(t_mgroup);

	return ret;
}
#pragma endregion definition of functions that load from other file formats

#pragma region friend_functions
void replace_node_data(mode* dest, mode* src)
{
	dest->nodes_list.assign(src->nodes_list.cbegin(), src->nodes_list.cend());
}
void replace_marker_stuff(mode* dest, mode* src)
{
	dest->markers_groups_list.assign(src->markers_groups_list.cbegin(), src->markers_groups_list.cend());
}
#pragma endregion
permutations::permutations()
{
	name = "";
	L1 = L2 = L3 = L4 = L5 = L6 = -1;
}
regions::regions()
{
	name = "";
}
void section_data::Calculate_vertex_normal_tangent_and_binormal()
{
	///vector<vector3d> normal_list;
	vector<vector3d> tangent_list;
	vector<vector3d> binormal_list;
	vector<int> count_list;

	///normal_list.resize(vertex_list.size(), { 0,0,0 });
	tangent_list.resize(vertex_list.size(), { 0,0,0 });
	binormal_list.resize(vertex_list.size(), { 0,0,0 });
	count_list.resize(vertex_list.size(), 0);

	for (int parts_iter = 0; parts_iter < parts_list.size(); parts_iter++)
	{
		parts& t_part = parts_list[parts_iter];
		
		for (int tria_iter = 0; tria_iter < t_part.face_list.size(); tria_iter++)
		{
			triangle_face& t_face = t_part.face_list[tria_iter];

			auto v0 = vertex_list[t_face.v0];
			auto v1 = vertex_list[t_face.v1];
			auto v2 = vertex_list[t_face.v2];

			auto x0 = v1.pos.x - v0.pos.x;
			auto x1 = v2.pos.x - v0.pos.x;
			auto y0 = v1.pos.y - v0.pos.y;
			auto y1 = v2.pos.y - v0.pos.y;
			auto z0 = v1.pos.z - v0.pos.z;
			auto z1 = v2.pos.z - v0.pos.z;

			auto s0 = v1.tex_cord.x - v0.tex_cord.x;
			auto s1 = v2.tex_cord.x - v0.tex_cord.x;
			auto t0 = v1.tex_cord.y - v0.tex_cord.y;
			auto t1 = v2.tex_cord.y - v0.tex_cord.y;

			auto r = s0 * t1 - s1 * t0;
			if (r == 0)
				continue;

			r = 1.0f / r;

			auto ni = y0*z1 - y1*z0;
			auto nj = -x0*z1 + x1*z0;
			auto nk = x0*y1 - x1*y0;
			auto n_len = sqrt(ni*ni + nj*nj + nk*nk);

			auto bi = -(s0 * x1 - s1 * x0) * r;
			auto bj = -(s0 * y1 - s1 * y0) * r;
			auto bk = -(s0 * z1 - s1 * z0) * r;
			auto b_len = sqrt(bi*bi + bj*bj + bk*bk);

			auto ti = (t1 * x0 - t0 * x1) * r;
			auto tj = (t1 * y0 - t0 * y1) * r;
			auto tk = (t1 * z0 - t0 * z1) * r;
			auto t_len = sqrt(ti*ti + tj*tj + tk*tk);

			///vector3d normal = { ni,nj,nk };
			vector3d binormal = { bi,bj,bk };
			vector3d tangent = { ti,tj,tk };

			///normal = normal*(1.0f / n_len);
			binormal = binormal*(1.0f / b_len);
			tangent = tangent*(1.0f / t_len);

			///normal_list[t_face.v0] = normal_list[t_face.v0] + normal;
			binormal_list[t_face.v0] = binormal_list[t_face.v0] + binormal;
			tangent_list[t_face.v0] = tangent_list[t_face.v0] + tangent;
			count_list[t_face.v0]++;

			///normal_list[t_face.v1] = normal_list[t_face.v1] + normal;
			binormal_list[t_face.v1] = binormal_list[t_face.v1] + binormal;
			tangent_list[t_face.v1] = tangent_list[t_face.v1] + tangent;
			count_list[t_face.v1]++;

			///normal_list[t_face.v2] = normal_list[t_face.v2] + normal;
			binormal_list[t_face.v2] = binormal_list[t_face.v2] + binormal;
			tangent_list[t_face.v2] = tangent_list[t_face.v2] + tangent;
			count_list[t_face.v2]++;
		}
	}
	for (int i = 0; i < vertex_list.size(); i++)
	{
		if (count_list[i] != 0)
		{
			///vertex_list[i].normal = normal_list[i] * (1.0f / count_list[i]);
			///vertex_list[i].normal.normalize();
			vertex_list[i].binormal = binormal_list[i] * (1.0f / count_list[i]);
			vertex_list[i].binormal.normalize();
			vertex_list[i].tangent = tangent_list[i] * (1.0f / count_list[i]);
			vertex_list[i].tangent.normalize();
		}
	}
}
//Does a part/material based redundant vertex removal
void section_data::Remove_redundant_vertices()
{
	std::vector<RAW_vertex> new_vertex_list;				///new reduced vertex list that has to be added into section_data

	for (int parts_iter = 0; parts_iter < parts_list.size(); parts_iter++)
	{
		parts& current_part = parts_list[parts_iter];
		std::vector<RAW_vertex> current_part_vertex_list;	///vertex list pertaining to a part
		///retrieve vertices to be processed,generates 3 vertex per triangle
		for (int tri_iter = 0; tri_iter < current_part.face_list.size(); tri_iter++)
		{
			triangle_face& current_face = current_part.face_list[tri_iter];

			int temp = current_part_vertex_list.size();

			current_part_vertex_list.push_back(vertex_list[current_face.v0]);
			current_part_vertex_list.push_back(vertex_list[current_face.v1]);
			current_part_vertex_list.push_back(vertex_list[current_face.v2]);

			current_face.v0 = temp;
			current_face.v1 = temp + 1;
			current_face.v2 = temp + 2;
		}
		///call the actual cleaner function
		Remove_redundant_vertices(current_part.face_list, current_part_vertex_list, new_vertex_list.size());
		///add the reduced vertex list to the new section_data list
		new_vertex_list.insert(new_vertex_list.end(), current_part_vertex_list.begin(), current_part_vertex_list.end());
	}
	///assign to the section_data vertex list
	vertex_list.assign(new_vertex_list.begin(), new_vertex_list.end());
}
///actual clean up program
void section_data::Remove_redundant_vertices(vector<triangle_face>& face_list, vector<RAW_vertex>& vertex_list, int vertex_start_off, float threshold)
{
	std::vector<int> vertex_ref;
	vertex_ref.resize(vertex_list.size(), 0);			///list to store references to newly added or matched up vertices

	std::vector<RAW_vertex> new_vertex_list;			///new reduced vertex list based on per part analysis
	///well the same vertex has got different normal depending on the face it is utilized and therefore has to averaged
	std::vector<int> vertex_ref_count;					///list to store no instance the vertex has been referred

	for (int vertex_iter = 0; vertex_iter < vertex_list.size(); vertex_iter++)
	{
		RAW_vertex& current_vertex = vertex_list[vertex_iter];
		///look for it
		int match_index = -1;
		for (int new_ver_iter = 0; new_ver_iter < new_vertex_list.size(); new_ver_iter++)
		{
			RAW_vertex& comp_vertex = new_vertex_list[new_ver_iter];

			if (abs(comp_vertex.pos.x - current_vertex.pos.x) < threshold)
			{
				if (abs(comp_vertex.pos.y - current_vertex.pos.y) < threshold)
				{
					if (abs(comp_vertex.pos.z - current_vertex.pos.z) < threshold);
					else continue;
				}
				else continue;
			}
			else continue;

			bool skip = false;
			for (int i = 0; i < 4; i++)
			{
				if (comp_vertex.nodes[i].index == current_vertex.nodes[i].index);
				else
				{
					skip = true;
					break;
				}
				if (abs(comp_vertex.nodes[i].weight - current_vertex.nodes[i].weight) < threshold);
				else
				{
					skip = true;
					break;
				}
			}
			if (skip)
				continue;
			if (abs(comp_vertex.tex_cord.x - current_vertex.tex_cord.x) <threshold)
			{
				if (abs(comp_vertex.tex_cord.y - current_vertex.tex_cord.y) < threshold);
				else continue;
			}
			else continue;
			///found our vertex
			match_index = new_ver_iter;
			break;
		}
		if (match_index == -1)
		{
			///no match,new vertex
			match_index = new_vertex_list.size();
			vertex_ref_count.push_back(1);							///add a new entry to refs count(for averaging purpose)
			new_vertex_list.push_back(current_vertex);				///add the new vertex
			vertex_ref[vertex_iter] = match_index;					///save the new reference for the vertex in the new list
		}
		else
		{
			///match found !!
			vertex_ref_count[match_index]++;						///increase ref count
			new_vertex_list[match_index].normal = new_vertex_list[match_index].normal + current_vertex.normal;///for averaging purpose
			vertex_ref[vertex_iter] = match_index;					///save the new reference for the duplicate vertex
		}
	}
	///averaging normals
	for (int i = 0; i < new_vertex_list.size(); i++)
		new_vertex_list[i].normal = new_vertex_list[i].normal*(1.0f / vertex_ref_count[i]);
	///now assign our new vertex list
	vertex_list.assign(new_vertex_list.begin(), new_vertex_list.end());
	///now to fix the vertex reference present in the triangles
	for (int tri_iter = 0; tri_iter < face_list.size(); tri_iter++)
	{
		triangle_face& current_face = face_list[tri_iter];

		int v0 = current_face.v0;
		int v1 = current_face.v1;
		int v2 = current_face.v2;

		current_face.v0 = vertex_start_off + vertex_ref[v0];
		current_face.v1 = vertex_start_off + vertex_ref[v1];
		current_face.v2 = vertex_start_off + vertex_ref[v2];
	}
}