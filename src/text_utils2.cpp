#include "text_utils.h"
#include <algorithm>

std::string removeStateSuffixes(const std::string& text){
  std::string result=text;
  size_t commaPos=result.rfind(',');
  if(commaPos!=std::string::npos) result=result.substr(0,commaPos);
  std::string upper=result;
  std::transform(upper.begin(),upper.end(),upper.begin(),::toupper);
  const std::vector<std::string> patterns={
    " ALABAMA"," ALASKA"," ARIZONA"," ARKANSAS"," CALIFORNIA"," COLORADO",
    " CONNECTICUT"," DELAWARE"," FLORIDA"," GEORGIA"," HAWAII"," IDAHO",
    " ILLINOIS"," INDIANA"," IOWA"," KANSAS"," KENTUCKY"," LOUISIANA",
    " MAINE"," MARYLAND"," MASSACHUSETTS"," MICHIGAN"," MINNESOTA",
    " MISSISSIPPI"," MISSOURI"," MONTANA"," NEBRASKA"," NEVADA",
    " NEW HAMPSHIRE"," NEW JERSEY"," NEW MEXICO"," NORTH CAROLINA",
    " NORTH DAKOTA"," OHIO"," OKLAHOMA"," OREGON"," PENNSYLVANIA",
    " RHODE ISLAND"," SOUTH CAROLINA"," SOUTH DAKOTA"," TENNESSEE"," TEXAS",
    " UTAH"," VERMONT"," VIRGINIA"," WEST VIRGINIA",
    " WISCONSIN"," WYOMING"," DISTRICT OF COLUMBIA"," US"," USA"," UNITED STATES"
  };
  for(const auto& p:patterns){
    size_t pos=upper.rfind(p);
    if(pos!=std::string::npos && pos+p.length()==upper.length()){
      result=result.substr(0,pos);
      upper=upper.substr(0,pos);
    }
  }
  while(!result.empty() && (result.back()==' '||result.back()=='\t')) result.pop_back();
  return result;
}

