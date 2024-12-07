// void TCPSocket::sendBackN(uint8_t *dataStream, uint32_t dataSize,
//                           const string &destIP, uint16_t destPort, uint32_t startingSeqNum)
// {
//     sh->setDataStream(dataStream, dataSize, startingSeqNum, port, destPort);
//     cout << "Awal: " << sh->getCurrentSeqNum() << " " << sh->getCurrentAckNum() << endl;
//     vector<thread> threads;
//     std::atomic<bool> retry(false);
//     bool endOfSegBuffer = false;
//     while (true)
//     {
//         // cout << "SeqNum and AckNum: " << sh->getCurrentSeqNum() << " " << sh->getCurrentAckNum() << endl;
//         while (sh->getCurrentSeqNum() - sh->getCurrentAckNum() < sh->getWindowSize())
//         {
//             Segment *seg = sh->advanceWindow(1);
//             if (seg == nullptr)
//             {
//                 // cout << "SeqNum and AckNum: " << sh->getCurrentSeqNum() << " " << sh->getCurrentAckNum() << endl;

//                 // cout << "END BUFFER" << endl;
//                 endOfSegBuffer = true;
//                 break;
//             }
//             // cout<<"Luar thread: "<<endl;
//             // printSegment(*seg);
//             threads.emplace_back([this, seg = *seg, destIP, destPort, startingSeqNum, &retry]()
//                                  {
//                 try{
//                     // cout<<"Dalam thread: "<<endl;
//                     // printSegment(seg);
//                     std::cout << OUT << brackets(status_strings[(int)status]) << brackets("Seq " + std::to_string(seg.seqNum-startingSeqNum)) << brackets("S=" + std::to_string(seg.seqNum)) << "Sent" << endl;
//                     sendSegment(seg,destIP,destPort);
//                     Message result = consumeBuffer(destIP,destPort,0,seg.seqNum+1,ACK_FLAG,1);
//                     commandLine('i', "[Established] [A=" + std::to_string(result.segment.ackNum) + "] Received ACK request from " + result.ip + ":" + std::to_string(result.port));
//                     // printSegment(seg);
//                     sh->ackWindow(seg.seqNum);
//                 }catch (const std::runtime_error &e){
//                     std::cout << OUT << brackets("TIMEOUT") << brackets("Seq " + std::to_string(seg.seqNum-startingSeqNum)) << brackets("S=" + std::to_string(seg.seqNum)) << "Timeout" << endl;
//                     if(sh->getCurrentAckNum()+1 == seg.seqNum){
//                         retry = true;
//                         sh->goBackWindow();
//                     }
//                 } });
//         }
//         if (sh->getCurrentAckNum() == sh->getCurrentSeqNum() && endOfSegBuffer)
//         {
//             break;
//         }

//         if (retry)
//         {
//             for (auto &t : threads)
//             {
//                 if (t.joinable())
//                 {
//                     t.join();
//                 }
//             }
//             threads.clear();
//             retry = false;
//         }
//         // cout << "SeqNum and AckNum: "<<sh->getCurrentSeqNum() << " " << sh->getCurrentAckNum() << endl;
//     }
//     for (auto &t : threads)
//     {
//         if (t.joinable())
//         {
//             t.join();
//         }
//     }
//     threads.clear();
//     std::cout << OUT << brackets(status_strings[(int)status]) << "All segments sent to " << destIP << ":" << destPort << endl;
// }

// void TCPSocket::receiveBackN(vector<Segment> &resBuffer, string destIP, uint16_t destPort, uint32_t seqNum)
// {
//     int i = 0;
//     bool finished = false;
//     int limit = 0;
//     uint32_t seqNumIt = seqNum;
//     while (limit < 10 && !finished)
//     {
//         try
//         {
//             Message res = consumeBuffer(destIP, destPort);
//             // std::cout << std::to_string(res.segment.seqNum) << " " << std::to_string(seqNumIt) << endl;
//             if (res.segment.seqNum < seqNumIt)
//             {
//                 // cout << "SIni: " << endl;
//                 sendSegment(ack(0, res.segment.seqNum + 1), destIP, destPort);
//             }

//             if (res.segment.seqNum == seqNumIt)
//             {
//                 i++;
//                 resBuffer.push_back(res.segment);
//                 std::cout << IN << brackets(status_strings[(int)status]) << brackets("Seq " + std::to_string(i)) << brackets("S=" + std::to_string(res.segment.seqNum)) << "ACKed" << endl;
//                 seqNumIt++;
//                 sendSegment(ack(0, seqNumIt), destIP, destPort);
//                 std::cout << OUT << brackets(status_strings[(int)status]) << brackets("Seq " + std::to_string(i)) << brackets("A=" + std::to_string(seqNumIt)) << "Sent" << endl;

//                 if (res.segment.flags.ece == 1)
//                 {
//                     break;
//                 }
//             }
//         }
//         catch (const std::exception &e)
//         {
//             limit++;
//             commandLine('!', "[ERROR] [Established] " + std::string(e.what()));
//         }
//     }
// }

// #include "segment_handler.hpp"

// SegmentHandler::SegmentHandler()
//     : windowSize(5), currentSeqNum(0), currentAckNum(0),
//       dataStream(nullptr), dataSize(0), dataIndex(0) {}

// SegmentHandler::~SegmentHandler() {}

// void SegmentHandler::generateSegments(uint32_t startingSeqNum, uint16_t sourcePort, uint16_t destPort)
// {
//   for (Segment &seg : segmentBuffer)
//   {
//     delete[] seg.payload;
//   }
//   segmentBuffer.clear();
//   cout << "startingSeqNum " << std::to_string(startingSeqNum) << endl;
//   // Hitung jumlah segment, hasil floor dataSize / MAX_PAYLOAD_SIZE
//   uint32_t numSegments = (dataSize + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;

//   segmentBuffer.resize(numSegments);

//   for (uint32_t i = 0; i < numSegments; i++)
//   {
//     uint32_t payloadSize = min(MAX_PAYLOAD_SIZE, dataSize - i * MAX_PAYLOAD_SIZE);
//     segmentBuffer.emplace_back();
//     Segment &seg = segmentBuffer.back();
//     seg.payload = new uint8_t[payloadSize];
//     memcpy(seg.payload, static_cast<uint8_t *>(dataStream) + i * MAX_PAYLOAD_SIZE, payloadSize);
//     seg.window = windowSize;
//     seg.payloadSize = payloadSize;
//     seg.checksum = 0;
//     seg.seqNum = startingSeqNum + i;
//   }

//   dataIndex = numSegments - 1;

//   if (dataSize > 0)
//     segmentBuffer[numSegments - 1].flags.ece = 1;

//   // for (int i = 0; i < segmentBuffer.size(); i++)
//   // {
//   //   cout << segmentBuffer[i].seqNum << endl;
//   // }
// }

// void SegmentHandler::setDataStream(uint8_t *dataStream, uint32_t dataSize, uint32_t startingSeqNum, uint16_t sourcePort, uint16_t destPort)
// {
//   this->dataStream = dataStream;
//   this->dataSize = dataSize;
//   currentSeqNum = startingSeqNum - 1;
//   currentAckNum = startingSeqNum - 1;

//   generateSegments(startingSeqNum, sourcePort, destPort);
//   cout << "Setelah generate segment: ";
// }

// uint8_t SegmentHandler::getWindowSize() { return this->windowSize; }

// Segment *SegmentHandler::advanceWindow(uint8_t size)
// {
//   lock_guard<mutex> lock(mtx);
//   if (dataIndex + 1 >= segmentBuffer.size())
//   {
//     // cout << "ADVANCE WINDOW NULL\nBEFORE: " << currentSeqNum << ", AFTER: " << currentSeqNum + size <<"\nCUR-ACK-NUM: "<< currentAckNum<<endl;
//     return nullptr;
//   }
//   else
//   {
//     // std::cout << "Dalam advanceWindow: " << dataIndex << " " << segmentBuffer.size() << std::endl;
//     // cout << "ADVANCE WINDOW\nCUR-SEQ-NUM BEFORE: " << currentSeqNum << ", AFTER: " << currentSeqNum + size << endl;
//     currentSeqNum += size;
//     dataIndex += 1;
//   }

//   return &segmentBuffer[dataIndex];
// }

// void SegmentHandler::ackWindow(uint32_t seqNum)
// {
//   lock_guard<mutex> lock(mtx);
//   // cout << "ACK WINDOW\nCUR-ACK-NUM BEFORE: " << currentAckNum << ", AFTER: " << currentAckNum + size << endl;
//   if (seqNum > currentAckNum)
//   {
//     // cout << "ACK WINDOW\nCUR-ACK-NUM BEFORE: " << currentAckNum << ", AFTER: " << std::to_string(seqNum) << endl;
//     currentAckNum = seqNum;
//   }else{
//         // cout << "ACK WINDOW\nSTILL: " << currentAckNum <<", input "<<std::to_string(seqNum)<< endl;

//   }
// }

// uint32_t SegmentHandler::getCurrentSeqNum()
// {
//   lock_guard<mutex> lock(mtx);
//   return currentSeqNum;
// }

// uint32_t SegmentHandler::getCurrentAckNum()
// {
//   lock_guard<mutex> lock(mtx);
//   return currentAckNum;
// }

// void SegmentHandler::goBackWindow()
// {
//   lock_guard<mutex> lock(mtx);
//   // cout << "ADVANCE WINDOW\nBEFORE: " << currentSeqNum << ", AFTER: " << currentSeqNum + size << endl;
//   // cout << "GOBACKWINDOW BEFORE: " << dataIndex << " " << currentSeqNum << endl;
//   dataIndex -= (currentSeqNum - currentAckNum);
//   currentSeqNum = currentAckNum;
//   // cout << "GOBACKWINDOW AFTER: " << dataIndex << " " << currentSeqNum << endl;
// }
