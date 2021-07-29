#include <iostream>
#include <vector>
#include <string>
#include "tinyxml2.h"
using namespace tinyxml2;
using namespace std;
class Device_XxPdo_Entry {
public:
	//Attributes:
	//ChildNodes:
	string Index;
	string SubIndex;
	string BitLen;
	string Name;
	string DataType;
	bool Init_Device_XxPdo_Entry(XMLElement* EntryNode) {
		XMLElement* IndexNode = EntryNode->FirstChildElement("Index");
		XMLElement* SubIndexNode = EntryNode->FirstChildElement("SubIndex");
		XMLElement* BitLenNode = EntryNode->FirstChildElement("BitLen");
		XMLElement* NameNode = EntryNode->FirstChildElement("Name");
		XMLElement* DataTypeNode = EntryNode->FirstChildElement("DataType");
		if (IndexNode == NULL || IndexNode->GetText() == 0) this->Index = "NULL";
		else this->Index = IndexNode->GetText();
		if (SubIndexNode == NULL || SubIndexNode->GetText() == 0) this->SubIndex = "NULL";
		else this->SubIndex = SubIndexNode->GetText();
		if (BitLenNode == NULL || BitLenNode->GetText() == 0) this->BitLen = "NULL";
		else this->BitLen = BitLenNode->GetText();
		if (NameNode == NULL || NameNode->GetText() == 0) this->Name = "NULL";
		else this->Name = NameNode->GetText();
		if (DataTypeNode == NULL || DataTypeNode->GetText() == 0) this->DataType = "NULL";
		else this->DataType = DataTypeNode->GetText();
		return true;
	}
};
class Device_Type {
public:
	//Text
	string Text;
	//Attributes:
	string ProduceCode;
	string RevisionNo;
	string CheckRevisionNo;
	//ChildNodes:

	bool Init_Device_Type(XMLElement* TypeNode) {
		if (TypeNode->GetText() == 0) this->Text = "NULL";
		else this->Text = TypeNode->GetText();
		if (TypeNode->Attribute("ProduceCode") == 0) this->ProduceCode = "NULL";
		else this->ProduceCode = TypeNode->Attribute("ProduceCode");
		if (TypeNode->Attribute("RevisionNo") == 0) this->RevisionNo = "NULL";
		else this->RevisionNo = TypeNode->Attribute("RevisionNo");
		if (TypeNode->Attribute("CheckRevisionNo") == 0) this->CheckRevisionNo = "NULL";
		else this->CheckRevisionNo = TypeNode->Attribute("CheckRevisionNo");
		return true;
	}
};
class Device_Name {
public:
	//Text
	string Text;
	//Attributes:
	string LcId;
	//ChildNodes:

	bool Init_Device_Name(XMLElement* NameNode) {
		if (NameNode->GetText() == 0) this->Text = "NULL";
		else this->Text = NameNode->GetText();
		if (NameNode->Attribute("LcId") == 0) this->LcId = "NULL";
		else this->LcId = NameNode->Attribute("LcId");
		return true;
	}
};
class Device_RxPdo {
private:
	bool Init_Entry(XMLElement* RxPdoNode) {
		XMLElement* EntryNode = RxPdoNode->FirstChildElement("Entry");
		while (EntryNode != NULL) {
			Device_XxPdo_Entry EntryElement;
			EntryElement.Init_Device_XxPdo_Entry(EntryNode);
			this->Entry.push_back(EntryElement);
			EntryNode = EntryNode->NextSiblingElement("Entry");
		}
		return true;
	}
public:
	//Attributes:
	string Fixed;
	string Mandatory;
	string Sm;
	//ChildNodes:
	string Index;
	string Name;
	vector<Device_XxPdo_Entry> Entry;

	bool Init_Device_RxPdo(XMLElement* RxPdoNode) {
		if (RxPdoNode->Attribute("Fixed") == 0) this->Fixed = "NULL";
		else this->Fixed = RxPdoNode->Attribute("Fixed");
		if (RxPdoNode->Attribute("Mandatory") == 0) this->Mandatory = "NULL";
		else this->Mandatory = RxPdoNode->Attribute("Mandatory");
		if (RxPdoNode->Attribute("Sm") == 0) this->Sm = "NULL";
		else this->Sm = RxPdoNode->Attribute("Sm");
		XMLElement* IndexNode = RxPdoNode->FirstChildElement("Index");
		XMLElement* NameNode = RxPdoNode->FirstChildElement("Name");
		if (IndexNode == NULL || IndexNode->GetText() == 0) this->Index = "NULL";
		else this->Index = IndexNode->GetText();
		if (NameNode == NULL || NameNode->GetText() == 0) this->Name = "NULL";
		else this->Name = NameNode->GetText();

		this->Init_Entry(RxPdoNode);
		return true;
	}
};
class Device_TxPdo {
private:
	bool Init_Entry(XMLElement* TxPdoNode) {
		XMLElement* EntryNode = TxPdoNode->FirstChildElement("Entry");
		while (EntryNode != NULL) {
			Device_XxPdo_Entry EntryElement;
			EntryElement.Init_Device_XxPdo_Entry(EntryNode);
			this->Entry.push_back(EntryElement);
			EntryNode = EntryNode->NextSiblingElement("Entry");
		}
		return true;
	}
public:
	//Attributes:
	string Fixed;
	string Mandatory;
	string Sm;
	//ChildNodes:
	string Index;
	string Name;
	vector<Device_XxPdo_Entry> Entry;
	bool Init_Device_TxPdo(XMLElement* TxPdoNode) {
		if (TxPdoNode->Attribute("Fixed") == 0) this->Fixed = "NULL";
		else this->Fixed = TxPdoNode->Attribute("Fixed");
		if (TxPdoNode->Attribute("Mandatory") == 0) this->Mandatory = "NULL";
		else this->Mandatory = TxPdoNode->Attribute("Mandatory");
		if (TxPdoNode->Attribute("Sm") == 0) this->Sm = "NULL";
		else this->Sm = TxPdoNode->Attribute("Sm");
		XMLElement* IndexNode = TxPdoNode->FirstChildElement("Index");
		XMLElement* NameNode = TxPdoNode->FirstChildElement("Name");
		if (IndexNode == NULL || IndexNode->GetText() == 0) this->Index = "NULL";
		else this->Index = IndexNode->GetText();
		if (NameNode == NULL || NameNode->GetText() == 0) this->Name = "NULL";
		else this->Name = NameNode->GetText();

		this->Init_Entry(TxPdoNode);
		return true;
	}
};
class Devices_Device {
private:
	bool Init_Type(XMLElement* DeviceNode) {
		XMLElement* TypeNode = DeviceNode->FirstChildElement("Type");
		if (TypeNode == NULL) {
			cout << "cannot find Type node!" << endl;
			return false;
		}
		this->Type.Init_Device_Type(TypeNode);
		return true;
	}
	bool Init_Name(XMLElement* DeviceNode) {
		XMLElement* NameNode = DeviceNode->FirstChildElement("Name");
		if (NameNode == NULL) {
			cout << "cannot find Name node!" << endl;
			return false;
		}
		this->Name.Init_Device_Name(NameNode);
		return true;
	}
	bool Init_RxPdo(XMLElement* DeviceNode) {
		XMLElement* RxPdoNode = DeviceNode->FirstChildElement("RxPdo");
		if (RxPdoNode == NULL) {
			cout << "cannot find RxPdo node!" << endl;
			return false;
		}
		while (RxPdoNode != NULL) {
			Device_RxPdo RxPdoElement;
			RxPdoElement.Init_Device_RxPdo(RxPdoNode);
			this->RxPdo.push_back(RxPdoElement);
			RxPdoNode = RxPdoNode->NextSiblingElement("RxPdo");
		}
		return true;
	}
	bool Init_TxPdo(XMLElement* DeviceNode) {
		XMLElement* TxPdoNode = DeviceNode->FirstChildElement("TxPdo");
		if (TxPdoNode == NULL) {
			cout << "cannot find TxPdo node!" << endl;
			return false;
		}
		while (TxPdoNode != NULL) {
			Device_TxPdo TxPdoElement;
			TxPdoElement.Init_Device_TxPdo(TxPdoNode);
			this->TxPdo.push_back(TxPdoElement);
			TxPdoNode = TxPdoNode->NextSiblingElement("TxPdo");
		}
		return true;
	}
public:
	//Attributes:
	//ChildNodes:
	Device_Type Type;
	Device_Name Name;
	vector<Device_TxPdo> TxPdo;
	vector<Device_RxPdo> RxPdo;
	bool Init_Devices_Device(XMLElement* DeviceNode) {
		if (this->Init_Type(DeviceNode) == false) cout << "init Type failed!" << endl;
		if (this->Init_Name(DeviceNode) == false) cout << "init Name failed!" << endl;
		if (this->Init_RxPdo(DeviceNode) == false) cout << "init RxPdo failed!" << endl;
		if (this->Init_TxPdo(DeviceNode) == false) cout << "init TxPdo failed!" << endl;
		return true;
	}
};
class Descriptions_Devices {
private:
	bool Init_Device(XMLElement* DevicesNode) {
		XMLElement* DeviceNode = DevicesNode->FirstChildElement("Device");
		if (DeviceNode == NULL) {
			cout << "cannot find Device node!" << endl;
			return false;
		}
		while (DeviceNode != NULL) {
			Devices_Device DeviceElement;
			DeviceElement.Init_Devices_Device(DeviceNode);
			this->Device.push_back(DeviceElement);
			DeviceNode = DeviceNode->NextSiblingElement("Device");
		}
		return true;
	}
public:
	//ChildNodes:
	vector<Devices_Device> Device;

	bool Init_Descriptions_Devices(XMLElement* DevicesNode) {
		if (this->Init_Device(DevicesNode) == false) cout << "init Device failed!" << endl;
		return true;
	}
};

void printf_EtherCATInfoNode(XMLElement* EtherCATInfoNode){
	XMLElement* DescriptionsNode = EtherCATInfoNode->FirstChildElement("Descriptions");
	XMLElement* DevicesNode = DescriptionsNode->FirstChildElement("Devices");
	XMLElement* DeviceNode = DevicesNode->FirstChildElement("Device");

	Devices_Device tmp;
	if (tmp.Init_Devices_Device(DeviceNode)) {
		//cout << "prase successed" << endl;
		int TxPdoNum = tmp.TxPdo.size();
		cout<<"----------------type----------------"<<endl;
		cout<<"ProductCode:"<<tmp.Type.ProduceCode<<endl;
		cout<<"RevisionNo:"<<tmp.Type.RevisionNo<<endl;
		cout<<"RevisionNo:"<<tmp.Name.Text<<endl;

		cout<<"----------------TxPdo----------------"<<endl;
		for (int i = 0; i < TxPdoNum; i++) {
			cout << "Index: " << tmp.TxPdo[i].Index << endl;
			cout << "Name: " << tmp.TxPdo[i].Name << endl;
			int EntryNum = tmp.TxPdo[i].Entry.size();
			for (int j = 0; j < EntryNum; j++) {
				cout << "Entry" << j + 1 << ": " << "    ";
				cout << "Index: " << tmp.TxPdo[i].Entry[j].Index <<"    ";
				cout << "SubIndex: " << tmp.TxPdo[i].Entry[j].SubIndex << "    ";
				cout << "BitLen: " << tmp.TxPdo[i].Entry[j].BitLen << "    ";
				cout << "Name: " << tmp.TxPdo[i].Entry[j].Name << "    ";
				cout << "DataType: " << tmp.TxPdo[i].Entry[j].DataType << endl;
			}
			cout << endl;
		}

		cout<<"----------------RxPdo----------------"<<endl;
		int RxPdoNum = tmp.RxPdo.size();
		for (int i = 0; i < RxPdoNum; i++) {
			cout << "Index: " << tmp.RxPdo[i].Index << endl;
			cout << "Name: " << tmp.RxPdo[i].Name << endl;
			int EntryNum = tmp.RxPdo[i].Entry.size();
			for (int j = 0; j < EntryNum; j++) {
				cout << "Entry" << j + 1 << ": " <<"    " ;
				cout << "Index: " << tmp.RxPdo[i].Entry[j].Index << "    ";
				cout << "SubIndex: " << tmp.RxPdo[i].Entry[j].SubIndex << "    ";
				cout << "BitLen: " << tmp.RxPdo[i].Entry[j].BitLen << "    ";
				cout << "Name: " << tmp.RxPdo[i].Entry[j].Name << "    ";
				cout << "DataType: " << tmp.RxPdo[i].Entry[j].DataType << endl;
			}
			cout << endl;
		}
	}
}

int main(int argc, char const *argv[]) {
	int server_num = 0;
	const char* xmlPath = argv[1];
	XMLDocument doc;
	if (doc.LoadFile(xmlPath) != 0) {
		cout << "load xml file failed!" << endl;
		return false;
	}
	//doc.Print()
	XMLElement* EtherCATInfoList = doc.RootElement();
	XMLElement* EtherCATInfoNode = EtherCATInfoList->FirstChildElement("EtherCATInfo");
	
	if (EtherCATInfoNode==NULL){
		EtherCATInfoNode = doc.RootElement();
		printf_EtherCATInfoNode(EtherCATInfoNode);
		server_num++;
	}
	else{
		while (EtherCATInfoNode!=NULL)
		{
			server_num++;
			cout<<"----------------------------SERVER"<<server_num<<"----------------------------"<<endl;
			printf_EtherCATInfoNode(EtherCATInfoNode);
			EtherCATInfoNode = EtherCATInfoNode->NextSiblingElement("EtherCATInfo");
		}
	}

	/*Descriptions_Devices Devices;
	Devices.Init_Descriptions_Devices(DevicesNode);
	int DeviceNum = Devices.Device.size();*/
	return 0;
}