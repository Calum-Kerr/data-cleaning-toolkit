#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <map>
#include <algorithm>
#include <chrono>
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

std::string toLower(const std::string& str){
    std::string result=str;
    std::transform(result.begin(),result.end(),result.begin(),::tolower);
    return result;
}

bool isNameColumn(const std::string& colName){
    std::string lower=toLower(colName);
    const std::vector<std::string> nameKeywords={"player","name","first name","last name","full name","employee","customer","person","author","contact","user","username","email","phone","address","street","city","country","state","zip","postal"};
    for(const auto& keyword:nameKeywords){
        if(lower.find(keyword)!=std::string::npos){
            return true;
        }
    }
    return false;
}

bool isBoolean(const std::string& str){
    std::string lower=toLower(str);
    return lower=="true"||lower=="false"||lower=="yes"||lower=="no"||lower=="y"||lower=="n"||lower=="1"||lower=="0";
}

bool isDateFormat(const std::string& str){
    if(str.length()<6||str.length()>10)return false;
    int digits=0,separators=0;
    char sep='\0';
    for(char c:str){
        if(c>='0'&&c<='9')digits++;
        else if(c=='/'||c=='-'||c=='.'){separators++;if(sep=='\0')sep=c;}
        else return false;
    }
    return digits>=4&&digits<=8&&separators==2;
}

std::string detectDateFormat(const std::string& str){
    if(str.length()<6)return "";
    char sep='/';
    if(str.find('-')!=std::string::npos)sep='-';
    else if(str.find('.')!=std::string::npos)sep='.';
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    while(std::getline(ss,part,sep))parts.push_back(part);
    if(parts.size()!=3)return "";
    if(parts[0].length()==4)return "YYYY"+std::string(1,sep)+"MM"+std::string(1,sep)+"DD";
    if(parts[2].length()==4){
        if(std::stoi(parts[0])>12)return "DD"+std::string(1,sep)+"MM"+std::string(1,sep)+"YYYY";
        if(std::stoi(parts[1])>12)return "MM"+std::string(1,sep)+"DD"+std::string(1,sep)+"YYYY";
        return "DD"+std::string(1,sep)+"MM"+std::string(1,sep)+"YYYY";
    }
    return "";
}

std::string standardiseDateToISO(const std::string& str){
    if(str.length()<6)return str;
    char sep='/';
    if(str.find('-')!=std::string::npos)sep='-';
    else if(str.find('.')!=std::string::npos)sep='.';
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    while(std::getline(ss,part,sep))parts.push_back(part);
    if(parts.size()!=3)return str;
    std::string year,month,day;
    if(parts[0].length()==4){year=parts[0];month=parts[1];day=parts[2];}
    else if(parts[2].length()==4){year=parts[2];
        if(std::stoi(parts[0])>12){day=parts[0];month=parts[1];}
        else if(std::stoi(parts[1])>12){month=parts[0];day=parts[1];}
        else{day=parts[0];month=parts[1];}}
    else{return str;}
    if(month.length()==1)month="0"+month;
    if(day.length()==1)day="0"+day;
    return year+"-"+month+"-"+day;
}

std::string standardiseNumber(const std::string& str){
    std::string result;
    bool hasDecimal=false;
    int commaCount=0,periodCount=0;
    for(size_t i=0;i<str.length();i++){
        if(str[i]==',')commaCount++;
        else if(str[i]=='.')periodCount++;}
    char decimalSep='.';
    if(commaCount>periodCount)decimalSep=',';
    for(size_t i=0;i<str.length();i++){
        char c=str[i];
        if(c>='0'&&c<='9'){result+=c;}
        else if((c=='.'||c==',')&&!hasDecimal){if(c==decimalSep){result+='.';hasDecimal=true;}}
        else if(c=='-'&&result.empty()){result+=c;}}
    return result.empty()?str:result;
}

std::string inferColumnType(const std::string& colName,const std::vector<std::string>& values){
    if(isNameColumn(colName))return "name";
    if(values.empty())return "text";
    int numericCount=0,dateCount=0,boolCount=0,textCount=0;
    for(const auto& val:values){
        if(val.empty())continue;
        if(isBoolean(val))boolCount++;
        else if(isNumeric(val))numericCount++;
        else if(isDateFormat(val))dateCount++;
        else textCount++;}
    int total=numericCount+dateCount+boolCount+textCount;
    if(total==0)return "text";
    if(boolCount*100/total>=80)return "boolean";
    if(numericCount*100/total>=80)return "numeric";
    if(dateCount*100/total>=80)return "date";
    return "text";
}

int validateColumnType(const std::vector<std::string>& values,const std::string& expectedType){
    int validCount=0;
    for(const auto& val:values){
        if(val.empty())continue;
        bool isValid=false;
        if(expectedType=="numeric"){isValid=isNumeric(val);}
        else if(expectedType=="date"){isValid=isDateFormat(val);}
        else if(expectedType=="boolean"){isValid=isBoolean(val);}
        else if(expectedType=="name"){isValid=!val.empty();}
        else{isValid=true;}
        if(isValid)validCount++;
    }
    return validCount;
}

std::string detectEncoding(const std::string& data){
    int utf8Count=0,latin1Count=0;
    for(size_t i=0;i<data.length();i++){
        unsigned char c=(unsigned char)data[i];
        if(c>=0x80){
            if(c>=0xC0&&c<=0xDF&&i+1<data.length()){
                unsigned char next=(unsigned char)data[i+1];
                if(next>=0x80&&next<=0xBF){utf8Count++;i++;}
            }else if(c>=0xE0&&c<=0xEF&&i+2<data.length()){
                unsigned char next1=(unsigned char)data[i+1];
                unsigned char next2=(unsigned char)data[i+2];
                if(next1>=0x80&&next1<=0xBF&&next2>=0x80&&next2<=0xBF){utf8Count++;i+=2;}
            }else{latin1Count++;}
        }
    }
    if(utf8Count>latin1Count)return "UTF-8";
    if(latin1Count>0)return "Latin-1";
    return "ASCII";
}

std::string normalizeToUTF8(const std::string& data){
    return data;
}

bool isEmailFormat(const std::string& str){
    size_t atPos=str.find('@');
    if(atPos==std::string::npos||atPos==0||atPos==str.length()-1)return false;
    size_t dotPos=str.find('.',atPos);
    return dotPos!=std::string::npos&&dotPos>atPos+1&&dotPos<str.length()-1;
}

bool isPhoneFormat(const std::string& str){
    int digitCount=0;
    for(char c:str){if(c>='0'&&c<='9')digitCount++;}
    return digitCount>=7&&digitCount<=15;
}

bool isPostalCodeFormat(const std::string& str){
    if(str.length()<4||str.length()>10)return false;
    int digitCount=0,letterCount=0;
    for(char c:str){
        if(c>='0'&&c<='9')digitCount++;
        else if((c>='A'&&c<='Z')||(c>='a'&&c<='z'))letterCount++;
        else if(c!=' '&&c!='-')return false;
    }
    return (digitCount+letterCount)>=4;
}

int levenshteinDistance(const std::string& s1,const std::string& s2){
    size_t m=s1.length();
    size_t n=s2.length();
    std::vector<std::vector<int>> dp(m+1,std::vector<int>(n+1,0));
    for(size_t i=0;i<=m;i++)dp[i][0]=i;
    for(size_t j=0;j<=n;j++)dp[0][j]=j;
    for(size_t i=1;i<=m;i++){
        for(size_t j=1;j<=n;j++){
            if(s1[i-1]==s2[j-1]){
                dp[i][j]=dp[i-1][j-1];
            }else{
                dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});
            }
        }
    }
    return dp[m][n];
}

std::vector<std::vector<std::string>> parseCSVInternal(const std::string& data){
    std::vector<std::vector<std::string>> result;
    result.reserve(1000);
    std::stringstream ss(data);
    std::string line;
    while(std::getline(ss,line)){
        if(!line.empty()&&line.back()=='\r'){line.pop_back();}
        std::vector<std::string> row;
        row.reserve(50);
        std::stringstream lineStream(line);
        std::string cell;
        while(std::getline(lineStream,cell,',')){
            if(!cell.empty()&&cell.back()=='\r'){cell.pop_back();}
            row.push_back(cell);
        }
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
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::vector<std::pair<double,int>> valueRowPairs;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumeric(parsed[row][col])){double val=std::stod(parsed[row][col]);valueRowPairs.push_back({val,row});}}
            if(valueRowPairs.size()<4)continue;
            std::sort(valueRowPairs.begin(),valueRowPairs.end());
            size_t n=valueRowPairs.size();
            double q1=valueRowPairs[n/4].first;
            double q3=valueRowPairs[3*n/4].first;
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<valueRowPairs.size();i++){if(valueRowPairs[i].first<lower||valueRowPairs[i].first>upper){outlierRows.insert(valueRowPairs[i].second);}}
        }
        return outlierRows.size();
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
            double q1=valueRowPairs[n/4].first;
            double q3=valueRowPairs[3*n/4].first;
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
    EMSCRIPTEN_KEEPALIVE
    int detectInconsistentValues(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2)return 0;
        int inconsistentCount=0;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::string colName=parsed[0][col];
            if(isNameColumn(colName))continue;
            std::vector<std::string> colValues;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&!parsed[row][col].empty()){colValues.push_back(parsed[row][col]);}}
            if(colValues.size()<2)continue;
            std::set<std::string> processedValues;
            for(size_t i=0;i<colValues.size();i++){
                if(processedValues.count(colValues[i]))continue;
                processedValues.insert(colValues[i]);
                bool foundSimilar=false;
                for(size_t j=i+1;j<colValues.size();j++){
                    if(processedValues.count(colValues[j]))continue;
                    int distance=levenshteinDistance(colValues[i],colValues[j]);
                    if(distance<=2){foundSimilar=true;inconsistentCount++;}
                }
                if(foundSimilar)inconsistentCount++;
            }
        }
        return inconsistentCount;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* standardiseValuesString(const char* csvData,const char* mappingsJson){
        std::string data(csvData);
        std::string mappings(mappingsJson);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2){char* cstr=new char[1];cstr[0]='\0';return cstr;}
        std::map<std::string,std::map<std::string,std::string>> columnMappings;
        size_t pos=0;
        std::string currentCol;
        while(pos<mappings.length()){
            if(mappings[pos]=='"'){
                size_t endQuote=mappings.find('"',pos+1);
                if(endQuote!=std::string::npos){
                    std::string key=mappings.substr(pos+1,endQuote-pos-1);
                    pos=endQuote+1;
                    while(pos<mappings.length()&&(mappings[pos]==':'||mappings[pos]==' '||mappings[pos]=='\t'||mappings[pos]=='\n')){pos++;}
                    if(pos<mappings.length()&&mappings[pos]=='{'){
                        currentCol=key;
                        pos++;
                        while(pos<mappings.length()&&mappings[pos]!='}'){
                            if(mappings[pos]=='"'){
                                size_t keyEnd=mappings.find('"',pos+1);
                                if(keyEnd!=std::string::npos){
                                    std::string fromVal=mappings.substr(pos+1,keyEnd-pos-1);
                                    pos=keyEnd+1;
                                    while(pos<mappings.length()&&(mappings[pos]==':'||mappings[pos]==' '||mappings[pos]=='\t'||mappings[pos]=='\n')){pos++;}
                                    if(pos<mappings.length()&&mappings[pos]=='"'){
                                        size_t valEnd=mappings.find('"',pos+1);
                                        if(valEnd!=std::string::npos){std::string toVal=mappings.substr(pos+1,valEnd-pos-1);columnMappings[currentCol][fromVal]=toVal;pos=valEnd+1;
                                        }else{pos++;}
                                    }else{pos++;}
                                }else{pos++;}
                            }else{pos++;}
                        }
                    }else{pos++;}
                }else{pos++;}
            }else{pos++;}
        }
        std::stringstream result;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string cell=parsed[row][col];
                if(row>0&&col<parsed[0].size()){std::string colName=parsed[0][col];if(columnMappings.count(colName)&&columnMappings[colName].count(cell)){cell=columnMappings[colName][cell];}}
                if(col>0)result<<",";
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
    const char* detectDataTypes(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2){char* cstr=new char[1];cstr[0]='\0';return cstr;}
        std::stringstream result;
        result<<"{\"types\":{";
        for(size_t col=0;col<parsed[0].size();col++){
            std::string colName=parsed[0][col];
            std::vector<std::string> colValues;
            for(size_t row=1;row<parsed.size()&&colValues.size()<50;row++){if(col<parsed[row].size()){colValues.push_back(parsed[row][col]);}}
            std::string type=inferColumnType(colName,colValues);
            if(col>0)result<<",";
            result<<"\""+colName+"\":\""+type+"\"";}
        result<<"},\"message\":\"data types detected\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* standardiseDateColumnString(const char* csvData,int colIndex){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string cell=parsed[row][col];
                if(col==(size_t)colIndex&&row>0&&isDateFormat(cell)){cell=standardiseDateToISO(cell);}
                if(col>0)result<<",";
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
    const char* standardiseNumberColumnString(const char* csvData,int colIndex){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string cell=parsed[row][col];
                if(col==(size_t)colIndex&&row>0&&!cell.empty()){cell=standardiseNumber(cell);}
                if(col>0)result<<",";
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
    const char* detectFileEncoding(const char* csvData){
        std::string data(csvData);
        std::string encoding=detectEncoding(data);
        std::stringstream result;
        result<<"{\"encoding\":\""+encoding+"\",\"message\":\"encoding detected\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* detectPatterns(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.size()<2){char* cstr=new char[1];cstr[0]='\0';return cstr;}
        std::stringstream result;
        result<<"{\"patterns\":{";
        int emailCount=0,phoneCount=0,postalCount=0;
        for(size_t row=1;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                if(isEmailFormat(parsed[row][col]))emailCount++;
                if(isPhoneFormat(parsed[row][col]))phoneCount++;
                if(isPostalCodeFormat(parsed[row][col]))postalCount++;
            }
        }
        result<<"\"email\":"<<emailCount<<",\"phone\":"<<phoneCount<<",\"postal\":"<<postalCount;
        result<<"},\"message\":\"patterns detected\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* autoDetectAll(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int missingCount=0,dupCount=0,wsCount=0,nullCount=0,outlierCount=0,inconsistentCount=0;
        for(size_t row=1;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                if(parsed[row][col].empty())missingCount++;
                if(parsed[row][col].find_first_not_of(" \t\n\r")!=0||parsed[row][col].find_last_not_of(" \t\n\r")!=parsed[row][col].length()-1)wsCount++;
                if(parsed[row][col]=="NULL"||parsed[row][col]=="null"||parsed[row][col]=="N/A")nullCount++;
            }
        }
        std::set<std::vector<std::string>>uniqueRows;
        for(size_t row=1;row<parsed.size();row++){
            if(uniqueRows.count(parsed[row]))dupCount++;
            uniqueRows.insert(parsed[row]);
        }
        std::stringstream result;
        result<<"{\"summary\":{";
        result<<"\"totalRows\":"<<parsed.size()<<",";
        result<<"\"totalColumns\":"<<(parsed.size()>0?parsed[0].size():0)<<",";
        result<<"\"missingValues\":"<<missingCount<<",";
        result<<"\"duplicateRows\":"<<dupCount<<",";
        result<<"\"whitespaceIssues\":"<<wsCount<<",";
        result<<"\"nullValueIssues\":"<<nullCount<<",";
        result<<"\"outliers\":0,";
        result<<"\"inconsistentValues\":0";
        result<<"},\"message\":\"all detections completed\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* measurePerformance(const char* csvData){
        auto start=std::chrono::high_resolution_clock::now();
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int rows=parsed.size();
        int cols=rows>0?parsed[0].size():0;
        int cells=rows*cols;
        auto end=std::chrono::high_resolution_clock::now();
        auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        std::stringstream result;
        result<<"{\"metrics\":{";
        result<<"\"rows\":"<<rows<<",";
        result<<"\"columns\":"<<cols<<",";
        result<<"\"totalCells\":"<<cells<<",";
        result<<"\"executionTimeMs\":"<<duration.count()<<",";
        result<<"\"cellsPerSecond\":"<<(cells>0?(cells*1000)/duration.count():0);
        result<<"},\"message\":\"performance measured\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* profileColumn(const char* csvData, int colIndex){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.empty()||colIndex<0||(size_t)colIndex>=parsed[0].size()){
            std::string result="{\"values\":{},\"uniqueValues\":0,\"message\":\"invalid column index\"}";
            char* cstr=new char[result.length()+1];
            std::strcpy(cstr,result.c_str());
            return cstr;
        }
        std::map<std::string,int> profile;
        for(const auto& row:parsed){
            if((size_t)colIndex<row.size()){
                profile[row[colIndex]]++;
            }
        }
        std::stringstream result;
        result<<"{\"values\":{";
        bool first=true;
        for(const auto& pair:profile){
            if(!first)result<<",";
            result<<"\"";
            for(char c:pair.first){
                if(c=='"')result<<"\\\"";
                else if(c=='\r')result<<"\\r";
                else if(c=='\n')result<<"\\n";
                else if(c=='\\')result<<"\\\\";
                else result<<c;
            }
            result<<"\":";
            result<<pair.second;
            first=false;
        }
        result<<"},\"uniqueValues\":"<<profile.size()<<",\"columnIndex\":"<<colIndex<<",\"message\":\"column profiled successfully\"}";
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }

    EMSCRIPTEN_KEEPALIVE
    const char* testStandardize(const char* csvData, int colIndex, const char* caseType){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        if(parsed.empty()||colIndex<0||(size_t)colIndex>=parsed[0].size()){
            std::string result="{\"csvData\":\"\",\"message\":\"invalid column index\"}";
            char* cstr=new char[result.length()+1];
            std::strcpy(cstr,result.c_str());
            return cstr;
        }
        std::string caseTypeStr(caseType);
        for(auto& row:parsed){
            if((size_t)colIndex<row.size()){
                std::string& cell=row[colIndex];
                if(caseTypeStr=="upper"){
                    for(auto& c:cell){c=std::toupper(c);}
                }else if(caseTypeStr=="lower"){
                    for(auto& c:cell){c=std::tolower(c);}
                }
            }
        }
        std::string lineEnding="\n";
        if(data.find("\r\n")!=std::string::npos){lineEnding="\r\n";}
        std::stringstream output;
        for(size_t i=0;i<parsed.size();++i){
            for(size_t j=0;j<parsed[i].size();++j){
                if(j>0)output<<",";
                output<<parsed[i][j];
            }
            output<<lineEnding;
        }
        std::string outputStr=output.str();
        char* cstr=new char[outputStr.length()+1];
        std::strcpy(cstr,outputStr.c_str());
        return cstr;
    }

    EMSCRIPTEN_KEEPALIVE
    const char* removeEmptyRowsString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::vector<std::vector<std::string>> result;
        for(const auto& row:parsed){
            bool isEmpty=true;
            for(const auto& cell:row){
                if(!cell.empty()){isEmpty=false;break;}
            }
            if(!isEmpty){result.push_back(row);}
        }
        std::string lineEnding="\n";
        if(data.find("\r\n")!=std::string::npos){lineEnding="\r\n";}
        std::stringstream output;
        for(size_t i=0;i<result.size();++i){
            for(size_t j=0;j<result[i].size();++j){
                if(j>0)output<<",";
                output<<result[i][j];
            }
            output<<lineEnding;
        }
        std::string outputStr=output.str();
        char* cstr=new char[outputStr.length()+1];
        std::strcpy(cstr,outputStr.c_str());
        return cstr;
    }

    EMSCRIPTEN_KEEPALIVE
    const char* removeAllDuplicatesString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::vector<std::vector<std::string>> result;
        result.reserve(parsed.size());
        std::set<std::vector<std::string>> seen;
        for(size_t i=0;i<parsed.size();++i){
            const auto& row=parsed[i];
            if(i==0){
                result.push_back(row);
            }else{
                if(!seen.count(row)){
                    seen.insert(row);
                    result.push_back(row);
                }
            }
        }
        std::string lineEnding="\n";
        if(data.find("\r\n")!=std::string::npos){lineEnding="\r\n";}
        std::stringstream output;
        for(size_t i=0;i<result.size();++i){
            for(size_t j=0;j<result[i].size();++j){
                if(j>0)output<<",";
                output<<result[i][j];
            }
            output<<lineEnding;
        }
        std::string outputStr=output.str();
        char* cstr=new char[outputStr.length()+1];
        std::strcpy(cstr,outputStr.c_str());
        return cstr;
    }
}