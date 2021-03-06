#include "Circuit_Tests.h"


#include "Circuit/Circuit.h"

//#include "MyAssert.h"
#include <fstream>
#include "Common.h"
#include "cryptoTools/Common/Log.h"
#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Crypto/AES.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "DebugCircuits.h"

using namespace osuCrypto;


#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
void swapEndian(BitVector& bv, u64 byteIdx)
{
	BitVector bb;

	bb.copy(bv, byteIdx * 8, 8);

	for (u64 i = 0; i < 8; ++i)
	{
		bv[byteIdx * 8 + i] = bb[7 - i];
	}
}

std::string byteSpaced(BitVector& bv)
{
	BitVector bb;
	std::stringstream ss;
	for (u64 i = 0; i < (bv.size() + 7) / 8; ++i)
	{
		bb.resize(0);
		bb.copy(bv, i * 8, std::min<u64>(8, bv.size() - (i + 1) * 8));
		ss << bb << " ";
	}

	return ss.str();
}


void Circuit_BrisRead_SHA_Test_Impl()
{
    // SHA test vectors
    // in = 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    //		 
    // in = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f
    // out = fc99a2df88f42a7a7bb9d18033cdc6a20256755f9d5b9a5044a9cc315abe84a7
    // 
    // in = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    // out = ef0c748df4da50a8d6c43c013edc3ce76c9d9fa9a1458ade56eb86c0a64492d2
    // 
    // in = 243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c89452821e638d01377be5466cf34e90c6cc0ac29b7c97c50dd3f84d5b5b5470917
    // out = cf0ae4eb67d38ffeb94068984b22abde4e92bc548d14585e48dca8882d7b09ce



    BitVector input(512);
    Circuit cir;
    std::fstream in;

	std::string file = testData == "" ? "./circuits/sha-256.txt" : testData + "/circuits/sha-256.txt";

    in.open(file);

    if (in.is_open() == false)
        throw UnitTestFail("failed to open file: " + file);

    cir.readBris(in, false);



    BitVector output;

    cir.evaluate(input);
    cir.translate(input, output);


    u8 SHA1_zero_data[4 * 8]{
        0xda, 0x56, 0x98 , 0xbe, 0x17, 0xb9, 0xb4, 0x69,
        0x62, 0x33, 0x57 , 0x99, 0x77, 0x9f, 0xbe, 0xca,
        0x8c, 0xe5, 0xd4 , 0x91, 0xc0, 0xd2, 0x62, 0x43,
        0xba, 0xfe, 0xf9 , 0xea, 0x18, 0x37, 0xa9, 0xd8
    };




    BitVector expectedOut((u8*)SHA1_zero_data, 256);

	for (u64 i = 0; i < output.size() / 8; ++i)
	{
		swapEndian(expectedOut, i);
	}

    if (expectedOut != output)
    {
        std::cout << "out  " << output << std::endl;
        std::cout << "outx " << expectedOut << std::endl;


		//for (u64 i = 0; i < output.size(); ++i)
		//{
		//	if (output[i] != expectedOut[i])
		//		std::cout << " d "<< i <<" act " << output[i]  << " vs " << expectedOut[i] << std::endl;

		//	if(cir.mOutputInverts[i] )
		//		std::cout << " o " << i << std::endl;

		//}
        throw UnitTestFail();
    }

	// doesnt work, not sure why
	//u8 SHA1_allOnes_data[4 * 8]{
	//	0xef, 0x0c, 0x74, 0x8d, 0xf4, 0xda, 0x50, 0xa8,
	//	0xd6, 0xc4, 0x3c, 0x01, 0x3e, 0xdc, 0x3c, 0xe7,
	//	0x6c, 0x9d, 0x9f, 0xa9, 0xa1, 0x45, 0x8a, 0xde,
	//	0x56, 0xeb, 0x86, 0xc0, 0xa6, 0x44, 0x92, 0xd2
	//};

	//BitVector expectedOut2((u8*)SHA1_allOnes_data, 256);

	//input.resize(256);
	//memset(input.data(), -1, 32);

	//std::cout << input << std::endl;

	//cir.evaluate(input);
	//cir.translate(input, output);



	//for (u64 i = 0; i < output.size() / 8; ++i)
	//{
	//	swapEndian(expectedOut2, i);
	//}

	//if (expectedOut2 != output)
	//{
	//	std::cout << "2 out  " << output << std::endl;
	//	std::cout << "2 outx " << expectedOut2 << std::endl;

	//	throw UnitTestFail();
	//}

}


void Circuit_BrisRead_AES_Test_Impl()
{
    Circuit cir;
    std::fstream in; 

	std::string file = testData == "" ? "./circuits/AES-non-expanded.txt" : testData + "/circuits/AES-non-expanded.txt";

    in.open(file);

    if (in.is_open() == false)
        throw UnitTestFail("failed to open file: " + file);

    if (in.is_open() == false)
        throw UnitTestFail();

    cir.readBris(in, true);

	block key = toBlock(3546, 876556456); 
    block data = toBlock(456234532, 324523423);
    block enc;
	memset(&data, 0, 8);

    AES keyShed(key);
    keyShed.ecbEncBlock(data, enc);

    BitVector labels, output;
    labels.reserve(cir.WireCount());

    labels.resize(256);
	//memset(labels.data(), 0xff, 16);

	memcpy(labels.data(), &data, 16);
	memcpy(labels.data() + 16, &key, 16);


	for (int i = 0; i < 32; ++i)
	{
		swapEndian(labels, i);
	}


	cir.evaluate(labels);
	cir.translate(labels, output);

    BitVector expected;
    expected.assign(enc);

	for (int i = 0; i < 16; ++i)
	{
		swapEndian(output, i);
	}


    if (expected != output )
    {

		std::cout << "out  " << byteSpaced(output) << "  " << output.hammingWeight() << std::endl;
        std::cout << "outx " << byteSpaced(expected) << "  " << expected.hammingWeight()<< std::endl;
        throw UnitTestFail();
    }
}



void Circuit_Gen_Adder32_Test_Impl()
{
    setThreadName("CP_Test_Thread");

    Circuit cir = AdderCircuit(32);

    BitVector labels, output;
    labels.resize(cir.WireCount());
    labels[2] = 1;
    labels[32 + 2] = 1;

    //std::cout << "in " << labels << std::endl;

    cir.evaluate(labels);

    //std::cout << "ev " << labels << std::endl;

    cir.translate(labels, output);

    BitVector expected(33);
    expected[3] = 1;

    for (u64 i = 0; i < 33; ++i)
    {
        if (expected[i] != output[i])
        {
            //std::cout << "ex " << expected << std::endl;
            //std::cout << "ac " << output << std::endl;
            throw UnitTestFail();

        }
    }
}

void Circuit_BrisRead_Adder32_Test_Impl()
{
    setThreadName("CP_Test_Thread");

	std::string file = testData == "" ? "./circuits/adder_32bit.txt" : testData + "/circuits/adder_32bit.txt";
	Circuit cir;
    std::fstream in;
    in.open(file);

    if (in.is_open() == false)
        throw UnitTestFail("failed to open file: " + file);

    cir.readBris(in);


    BitVector labels, output;
    labels.reserve(cir.WireCount());
    labels.resize(64);
    ((u32*)labels.data())[0] = (u32)-1;
    ((u32*)labels.data())[1] = (u32)1;

    //std::cout << "in " << labels << std::endl;

    cir.evaluate(labels);


    //std::cout << "ev " << labels << std::endl;


    cir.translate(labels, output);

    BitVector expected(33);
    expected[32] = 1;

    if (expected != output)
    {
        std::cout << "ex " << expected << std::endl;
        std::cout << "ac " << output << std::endl;
        throw UnitTestFail("output doesnt match expected");

    }
}

//
//
//void DagCircuit_BrisRead_Adder32_Test_Impl()
//{
//
//
//    DagCircuit dag;
//    std::fstream in;
//    in.open(testData + "/circuits/adder_32bit.txt");
//
//    if (in.is_open() == false)
//        throw UnitTestFail("failed to open file: " + testData + "/circuits/adder_32bit.txt");
//
//    dag.readBris(in);
//
//    Circuit c0;
//    dag.toCircuit(c0);
//
//    dag.removeInvertGates();
//
//    Circuit c1;
//    dag.toCircuit(c1);
//    PRNG prng(ZeroBlock);
//
//    BitVector labels, output0, output1;
//    labels.reserve(c0.WireCount());
//
//    for (u64 i = 0; i < 100; ++i)
//    {
//        u32 input0 = prng.get<i32>();
//        u32 input1 = prng.get<u32>();
//
//
//        labels.resize(64);
//        ((u32*)labels.data())[0] = input0;
//        ((u32*)labels.data())[1] = input1;
//
//
//        c0.evaluate(labels);
//        c0.translate(labels, output0);
//
//        labels.resize(64);
//        c1.evaluate(labels);
//        c1.translate(labels, output1);
//
//        if (output0 != output1)
//            throw UnitTestFail();
//
//
//    }
//    //	std::fstream out;
//    //	out.open("../../circuits/adder_32bit_out.txt",std::ios::trunc | std::ios::out);
//
//    //	dag.writeBris(out);
//}
//
//
//
//void DagCircuit_RandomReduce_Test_Impl()
//{
//
//
//    u64 inputSize = 10;
//    u64 numGates = 1000;
//    u64 outputSize = 100;
//
//    PRNG prng(ZeroBlock);
//
//    DagCircuit dag;
//    dag.mWireCount = inputSize + numGates;
//
//    dag.mInputCounts[0] = inputSize / 2;
//    dag.mInputCounts[1] = inputSize - dag.mInputCounts[0];
//
//    dag.mInputs.resize(inputSize);
//    for (u64 i = 0; i < inputSize; ++i)
//    {
//        dag.mInputs[i].mWireIdx = i;
//        dag.mNodes.push_back(&dag.mInputs[i]);
//    }
//    dag.mNonInvertGateCount = 0;
//    dag.mGates.resize(numGates);
//    for (u64 i = 0; i < numGates; ++i)
//    {
//        auto& gate = dag.mGates[i];
//
//
//        u64 input0 = prng.get<u64>() % dag.mNodes.size();
//        u64 input1 = prng.get<u64>() % dag.mNodes.size();
//
//        if (input1 == input0)
//            input1 = (input1 + 1) % dag.mNodes.size();
//
//        gate.mParents.push_back(dag.mNodes[input0]);
//        u8 gt = prng.get<u8>() % ((gate.mParents[0]->isInvert()) ? 3 : 2);
//
//        if (gt == 0)
//        {
//            dag.mNonInvertGateCount++;
//            gate.mType = GateType::And;
//            gate.mParents.push_back(dag.mNodes[input1]);
//        }
//        else if (gt == 1)
//        {
//            dag.mNonInvertGateCount++;
//            gate.mType = GateType::Xor;
//            gate.mParents.push_back(dag.mNodes[input1]);
//        }
//        else
//        {
//            gate.mType = GateType::na;
//        }
//
//        for (auto parent : gate.mParents)
//        {
//            parent->mChildren.push_back(&gate);
//        }
//
//        dag.mNodes.push_back(&gate);
//
//        gate.mWireIdx = i + inputSize;
//    }
//
//    for (auto iter = dag.mNodes.end() - outputSize; iter != dag.mNodes.end(); ++iter)
//    {
//        dag.mOutputs.emplace_back();
//        dag.mOutputs.back().mParent = *iter;
//    }
//
//    Circuit nonReduce;
//    dag.toCircuit(nonReduce);
//
//    if (nonReduce.Gates().size() != dag.mGates.size())
//        throw UnitTestFail();
//
//    dag.removeInvertGates();
//
//    Circuit reduced;
//    dag.toCircuit(reduced);
//
//
//    if (reduced.Gates().size() != dag.mNonInvertGateCount)
//        throw UnitTestFail();
//
//    for (u64 i = 0; i < 15; ++i)
//    {
//        BitVector input(inputSize);
//        BitVector out0, out1;
//        input.randomize(prng);
//
//        nonReduce.evaluate(input);
//        nonReduce.translate(input, out0);
//
//        reduced.evaluate(input);
//        reduced.translate(input, out1);
//
//        if (out0 != out1)
//            throw UnitTestFail();
//    }
//
//}