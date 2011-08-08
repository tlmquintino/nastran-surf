#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cassert>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/program_options.hpp>

struct point
{
  unsigned int id;
  unsigned int oldid;
  double x;
  double y;
  double z;
};


struct triag
{
  unsigned int p0;
  unsigned int p1;
  unsigned int p2;
};

struct quad
{
  unsigned int p0;
  unsigned int p1;
  unsigned int p2;
  unsigned int p3;
};

int main(int argc, char * argv[])
{
  // Declare the supported options.
  boost::program_options::options_description desc("allowed options");
  desc.add_options()
      ("help",  "produce help message")
      ("quads", "convert quads" )
      ("input,i", boost::program_options::value<std::string>() , "name of the input file" )
      ("output,o", boost::program_options::value<std::string>() , "name of the output file" );

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);


  if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
  }

  bool cquads = (bool) vm.count("quads");

  std::string input_file_name ("surfaces.dat");
  std::string output_file_name ("surfaces.txt");

  if (vm.count("input"))
  {
    input_file_name = vm["input"].as<std::string>();
  }
  if (vm.count("output"))
  {
    output_file_name = vm["output"].as<std::string>();
  }
  std::cout << "converting nastran file \'" << input_file_name << "\' -> " << output_file_name << std::endl;

	std::cout.precision(10);

	std::map < unsigned int, point > mpts;
    std::vector <triag> tv (0);
    std::vector <quad>  qv (0);
  
	std::ifstream fin;
	fin.open( input_file_name.c_str() );
	if ( !fin.is_open() )
	{
		std::cout << "could not open input file [" << input_file_name << "]" << std::endl;
		return 1;
	}
	
	size_t nodes  = 0;	
	size_t triags = 0;	
	size_t quads  = 0;	
	size_t line_count;
	
	std::string line, line2;
	std::string sid, sx, sy, sz;
	std::string s0, s1, s2, s3;
	
	point pt;
	triag tt;
	quad  qq;
	
	while( std::getline(fin,line) ) 
	{
		++line_count;
		// std::cout << "[ " << line << " ]" << std::endl;
		
		// read a coordinate
		if ( boost::algorithm::starts_with(line, "GRID*") )
		{
			++nodes;

			// read continuation line
			std::getline(fin,line2); ++line_count;
//			std::cout << "[ " << line2 << " ]" << std::endl;

			sid = line.substr(12,12);
			
			sx = line.substr(40,16);
			sy = line.substr(56,16);
			sz = line2.substr(8,16);
			// std::cout << "[ [" << sid << "] \'" << sx << "\' \'" << sy << "\' \'" << sz << "\' ]" << std::endl;
	
			// convert to numerical values and build the point data
			std::string all = sid + " " + sx + " " + sy + " "+ sz ; 

			std::istringstream iss (all);	
			iss >> pt.oldid >> pt.x >> pt.y >> pt.z;
	
		    pt.id = nodes;     /* 1-based indexes */

			// insert point into db
		    mpts[pt.oldid] = pt;

		    assert( mpts[pt.oldid].id != 0 );

            // std::cout << pt.oldid << " " << pt.x << " " << pt.y << " " << pt.z << std::endl;

		}

		if ( !cquads && boost::algorithm::starts_with(line, "CTRIA3") )
		{
			++triags;
			// std::cout << "[ " << line << " ]" << std::endl;
			
			s0 = line.substr(24,8);
			s1 = line.substr(32,8);
			s2 = line.substr(40,8);
			
			// std::cout << "[ \'" << s0 << "\' \'" << s1 << "\' \'" << s2 << "\' ]" << std::endl;

			// convert to numerical values and build the point data
			std::string all = s0 + " " + s1 + " " + s2; 
			std::istringstream iss (all);	
			
			iss >> tt.p0 >> tt.p1 >> tt.p2;

		    tv.push_back( tt );

		    assert( tv.size() == triags );
		
		}
		
		if ( cquads && boost::algorithm::starts_with(line, "CQUAD4") )
		{
			++quads;
			// std::cout << "[ " << line << " ]" << std::endl;
			
			s0 = line.substr(24,8);
			s1 = line.substr(32,8);
			s2 = line.substr(40,8);
			s3 = line.substr(48,8);
			
			// std::cout << "[ \'" << s0 << "\' \'" << s1 << "\' \'" << s2 << "\'" << s3 << "\' ]" << std::endl;

			// convert to numerical values and build the point data
			std::string all = s0 + " " + s1 + " " + s2 + " " + s3; 
			std::istringstream iss (all);	
			
			iss >> qq.p0 >> qq.p1 >> qq.p2 >> qq.p3;

		    qv.push_back( qq );

		    assert( qv.size() == quads );
		}
	}

  // write coordinates

	std::ofstream fout; 
	fout.open( output_file_name.c_str() );
	if ( !fout.is_open() )
	{
		std::cout << "could not open output file [" << output_file_name << "]" << std::endl;
		return 1;
	}

  fout << nodes << std::endl;
  std::map < unsigned int, point >::const_iterator itr = mpts.begin();
  std::map < unsigned int, point >::const_iterator end = mpts.end();

  fout.precision(10);
  fout.setf(std::ios::fixed,std::ios::floatfield);

  for ( ; itr != end; ++itr )
  {
    fout << ( 1000.0 * itr->second.x ) << " " << ( 1000.0 * (itr->second.y) ) << " " << ( 1000.0 * itr->second.z ) << "\n";
  }

if ( !cquads ) // write triags
{
  fout << triags << std::endl;

  std::vector<triag>::const_iterator ti   = tv.begin();
  std::vector<triag>::const_iterator tend = tv.end();
  for ( ; ti != tend; ++ti )
  {
    fout << mpts[ ti->p0 ].id  << " " << mpts[ ti->p1 ].id << " " << mpts[ ti->p2 ].id << " 0\n" ;
  }
}  


if ( cquads ) // write quads
{
  fout << quads << std::endl;

  std::vector<quad>::const_iterator qi   = qv.begin();
  std::vector<quad>::const_iterator qend = qv.end();
  for ( ; qi != qend; ++qi )
  {
    fout << mpts[ qi->p0 ].id  << " " << mpts[ qi->p1 ].id << " " << mpts[ qi->p2 ].id  << " " << mpts[ qi->p3 ].id << "\n" ;
  }
}

  return 0;
}
