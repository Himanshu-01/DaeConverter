#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include<vector>

//JMS Parser coded by Himanshu01
//Currently based as per the format supplied by General101-H2PC[8200,8210]
//8210 ->https://pastebin.com/NmZEekp0
namespace jms
{	
	//parsing functions
	void _parse_next(std::ifstream*, std::string&);
	void _parse_next(std::ifstream*, int&);
	void _parse_next(std::ifstream*, float&);

	//lame coding
	struct vector3D
	{
	public: float x;
			float y;
			float z;

			vector3D() {
				x = y = z = 0.0f;
			}
			//addition operator
			vector3D operator+(const vector3D& other)
			{
				x += other.x;
				y += other.y;
				z += other.z;

				return *this;
			}
			//scalar multiplication
			vector3D operator*(const float scalar)
			{
				x *= scalar;
				y *= scalar;
				z *= scalar;

				return *this;
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream, int version);
			//write to ofstream(text stream)
			void write(std::ofstream* jms_stream, int version);
	};
	struct vector4D
	{
	public: float x;
			float y;
			float z;
			float w;

			vector4D() {
				x = y = z = w = 0.0f;
			}
			//addition operator
			vector4D operator+(const vector4D& other)
			{
				x += other.x;
				y += other.y;
				z += other.z;
				w += other.w;

				return *this;
			}
			//scalar multiplication
			vector4D operator*(int scalar)
			{
				x *= scalar;
				y *= scalar;
				z *= scalar;
				w *= scalar;

				return *this;
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream, int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream, int version);
	};
	//node structure to encompass all possible varieties
	struct node
	{
	public: std::string name;
			int parent_node_index;
			int first_child_node_index;
			int sibling_node_index;
			vector4D rotation;
			vector3D position;

			node() {
				name = "";
				parent_node_index = first_child_node_index = sibling_node_index = -1;
				rotation = vector4D();
				position = vector3D();
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream,int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream,int version);
	};
	struct material
	{
	public: std::string name;
			std::string tif_path;//CE lame
			std::string LOD;
			std::string Permutation;
			std::string Region;

			material() {
				name = tif_path = LOD = Permutation = Region = "";
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream,int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream,int version);
	};
	struct marker
	{
	public: std::string name;
			int region;
			int parent_node;
			vector4D rotation;
			vector3D position;
			float radius;

			marker(){
				name = "";
				region = parent_node = -1;
				rotation = vector4D();
				position = vector3D();
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream,int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream,int version);
	};
	struct region
	{
	public: std::string name;

			region() {
				name = "";
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream,int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream,int version);
	};
	struct vertex
	{
	public:// int node0;//i dont know why is it for(maybe for CE)
		 vector3D position;
		 vector3D normal;
		 //h2v supports 4 nodes and weights
		 std::vector<int> node_indices;
		 std::vector<float> node_weights;
		 vector3D tex_cords;//[u,v,w]=[x,y,z]
		 bool has_secondary_coordinates;
		 vector3D secondary_tex_coords;//second channel,usually used of lightmapping purpose

		 vertex() {
			 has_secondary_coordinates = false;
			 position = normal = tex_cords = secondary_tex_coords = vector3D();
		 }
		 //read from ifstream(text interface)
		 void read(std::ifstream* jms_stream,int version);
		 //write to ofstream(text interface)
		 void write(std::ofstream* jms_stream,int version);
	};
	struct triangle
	{
	public: int region_index;
			int shader_index;
			std::vector<int> vertex_indices;//3 vertex-lol

			triangle() {
				region_index = shader_index = -1;
			}
			//read from ifstream(text interface)
			void read(std::ifstream* jms_stream, int version);
			//write to ofstream(text interface)
			void write(std::ofstream* jms_stream, int version);
	};
	class jms
	{
		///reads jms8200 from the specifed stream,begins just after the version number
		void read_jms8200(std::ifstream* jms_stream);
		///reads jms8210 from the specifed stream,begins just after the version number
		void read_jms8210(std::ifstream* jms_Stream);
		///writes jms8200 into the specified stream,following the version number
		void write_jms8200(std::ofstream* jms_stream);
		///writes jms8210 into the specified stream,following the version number
		void write_jms8210(std::ofstream* jms_stream);
		///fixes parent node reference for jms8200
		void fix_jms8200_parent_ref();
		///removes copy of same vertices
		void clean_redundant_vertices(float threshold = 0.001f);
	public:
		//structure implemented for version 8200 and 8210,cause i am only aware of that
		int version;
		int checksum;//i always keep it 0,dont know its purpose(def val 3251 mentioned somewhere)
		std::vector<node> node_list;
		std::vector<material> material_list;
		std::vector<marker> marker_list;
		std::vector<region> region_list;
		std::vector<vertex> vertex_list;
		std::vector<triangle> triangle_list;		

		/// default constructor 
		/// sets version to 8210
		jms();
		/// default constructor to initialise stuff
		/// arg0 the version number
		jms(int arg0);
		/// opens a jms file and read its contents
		/// file_loc the complete file path of the jms file
		jms(std::string file_loc);
		/// destructor
		~jms();
		/// outputs the jms file accordingly to the version
		/// file_loc the complete file path where the file has be saved
		void save_file(std::string file_loc);
	};
}
