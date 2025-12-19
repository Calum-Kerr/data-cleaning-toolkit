#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include <emscripten/emscripten.h>

bool isNumeric(const std::string& str){
    if(str.empty())return false;
    size_t start=0;
    if(str[0]=='-'||str[0]=='+')start=1;
    if(start>=str.length())return false;
    bool hasDecimal=false;
    for(size_t i=start;i<str.length();i++){
        if(str[i]=='.'){
            if(hasDecimal)return false;
            hasDecimal=true;
        }else if(str[i]<'0'||str[i]>'9'){
            return false;
        }
    }
    return true;
}

std::vector<std::vector<std::string>> parseCSVInternal(const std::string& data){
    std::vector<std::vector<std::string>> result;
    std::stringstream ss(data);
    std::string line;
    while(std::getline(ss,line)){
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        std::string cell;
        while(std::getline(lineStream,cell,',')){row.push_back(cell);}
        if(!row.empty()){result.push_back(row);}
    }
    return result;
}

extern "C"{
    EMSCRIPTEN_KEEPALIVE
    int parseCSV(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        return parsed.size();
    }
    EMSCRIPTEN_KEEPALIVE
    int detectMissing(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectDuplicates(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        int count=0;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(seen.count(rowStr)){count++;}
            else{seen.insert(rowStr);}
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    int cleanData(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        int cleanedCount=0;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(!seen.count(rowStr)){
                seen.insert(rowStr);
                cleanedCount++;
            }
        }
        return cleanedCount;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* cleanDataString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        std::stringstream result;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(!seen.count(rowStr)){
                seen.insert(rowStr);
                result<<rowStr<<"\n";
            }
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectWhitespace(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(!cell.empty()&&(cell.front()==' '||cell.back()==' '||cell.front()=='\t'||cell.back()=='\t')){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* trimWhitespaceString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                size_t start=cell.find_first_not_of(" \t");
                size_t end=cell.find_last_not_of(" \t");
                if(start!=std::string::npos){cell=cell.substr(start,end-start+1);}
                else{cell="";}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* toUpperCaseString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){if(c>='a'&&c<='z')c=c-32;}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* toLowerCaseString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){if(c>='A'&&c<='Z')c=c+32;}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectNullValues(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* standardiseNullValueString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){cell="";}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectOutliers(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2)return 0;
        int outlierCount=0;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::vector<double> values;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumeric(parsed[row][col])){double val=std::stod(parsed[row][col]);values.push_back(val);}}
            if(values.size()<4)continue;
            std::sort(values.begin(),values.end());
            size_t n=values.size();
            double q1_idx=(n+1)/4.0-1;
            double q3_idx=3*(n+1)/4.0-1;
            size_t q1_lower=(size_t)q1_idx;
            size_t q1_upper=q1_lower+1;
            double q1_frac=q1_idx-q1_lower;
            double q1=(q1_upper<n)?values[q1_lower]*(1-q1_frac)+values[q1_upper]*q1_frac:values[q1_lower];
            size_t q3_lower=(size_t)q3_idx;
            size_t q3_upper=q3_lower+1;
            double q3_frac=q3_idx-q3_lower;
            double q3=(q3_upper<n)?values[q3_lower]*(1-q3_frac)+values[q3_upper]*q3_frac:values[q3_lower];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(double val:values){if(val<lower||val>upper)outlierCount++;}
        }
        return outlierCount;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* removeOutliersString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2){std::string empty="";char* cstr=new char[1];cstr[0]='\0';return cstr;}
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::vector<std::pair<double,int>> valueRowPairs;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumeric(parsed[row][col])){double val=std::stod(parsed[row][col]);valueRowPairs.push_back({val,row});}}
            if(valueRowPairs.size()<4)continue;
            std::sort(valueRowPairs.begin(),valueRowPairs.end());
            size_t n=valueRowPairs.size();
            double q1_idx=(n+1)/4.0-1;
            double q3_idx=3*(n+1)/4.0-1;
            size_t q1_lower=(size_t)q1_idx;
            size_t q1_upper=q1_lower+1;
            double q1_frac=q1_idx-q1_lower;
            double q1=(q1_upper<n)?valueRowPairs[q1_lower].first*(1-q1_frac)+valueRowPairs[q1_upper].first*q1_frac:valueRowPairs[q1_lower].first;
            size_t q3_lower=(size_t)q3_idx;
            size_t q3_upper=q3_lower+1;
            double q3_frac=q3_idx-q3_lower;
            double q3=(q3_upper<n)?valueRowPairs[q3_lower].first*(1-q3_frac)+valueRowPairs[q3_upper].first*q3_frac:valueRowPairs[q3_lower].first;
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<valueRowPairs.size();i++){if(valueRowPairs[i].first<lower||valueRowPairs[i].first>upper){outlierRows.insert(valueRowPairs[i].second);}}
        }
        std::stringstream result;
        for(size_t i=0;i<parsed.size();i++){
            if(outlierRows.find(i)==outlierRows.end()){
                for(size_t j=0;j<parsed[i].size();j++){
                    if(j>0)result<<",";
                    result<<parsed[i][j];
                }
                result<<"\n";
            }
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
}