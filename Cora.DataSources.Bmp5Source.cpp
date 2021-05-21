/* Cora.DataSources.Bmp5Source.cpp

   Copyright (C) 2016, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 03 October 2016
   Last Change: Wednesday 06 March 2019
   Last Commit: $Date: 2019-10-18 13:19:36 -0600 (Fri, 18 Oct 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.Bmp5Source.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.TableFieldUri.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.PakBus.AesCipher.h"
#include "Csi.PakBus.Bmp5.Defs.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.DevConfig.PakbusSession.h"
#include "Csi.ArrayDimensions.h"
#include "Csi.RangeList.h"
#include "Csi.MaxMin.h"


namespace Cora
{
   namespace DataSources
   {
      namespace Bmp5SourceHelpers
      {
         /**
          * Defines an object that performs PakBus routing for the BMP5 data source type.
          */
         class SourceRouter: public Csi::PakBus::Router
         {
         private:
            /**
             * Specifies the source that owns this router.
             */
            Bmp5Source *source;

            /**
             * Identifies the timer that controls when we broadcast hello requests.
             */
            uint4 search_id;

            /**
             * Specifies the log associated with the source.
             */
            typedef Csi::SharedPtr<Csi::LogByte> log_handle;
            log_handle log;
            
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this router.
             */
            SourceRouter(Bmp5Source *source_):
               Router(source_->get_manager()->get_timer()),
               source(source_),
               search_id(0)
            {
               set_this_node_address(source->my_pakbus_address);
               set_is_leaf_node(true);
               if(source->pakbus_encryption_key.length() && source->pakbus_address)
                  set_cipher(source->pakbus_address, new Csi::PakBus::AesCipher(source->pakbus_encryption_key));
            }

            /**
             * Destructor
             */
            virtual ~SourceRouter()
            {
               if(search_id && timer != 0)
                  timer->disarm(search_id);
            }

            /**
             * Overloads the one shot timer notififcation in order to check to see if a hello
             * request should be broadcast.
             */
            virtual void onOneShotFired(uint4 id);

            /**
             * Starts the process of searching for a neighbour datalogger.
             */
            void start_search();

            /**
             * Ends the process of searching for a neighbour datalogger.
             */
            void end_search();

            /**
             * Sets the log for this router.
             */
            void set_log(log_handle log_)
            { log = log_; }

            /**
             * Overloads the base class version to handle the discovery of a new node.
             */
            virtual void on_link_change(uint2 node1, uint2 node2, Csi::PakBus::HopMetric hop_metric, link_change_type change);

            /**
             * Overloads the base class version to handle the case where the port has failed.
             */
            virtual void on_port_delivery_failure(
               Csi::PakBus::PortBase *port, uint2 physical_destination)
            {
               Router::on_port_delivery_failure(port, physical_destination);
               source->get_manager()->report_source_disconnect(source, ManagerClient::disconnect_connection_failed);
            }

            /**
             * Overloads the handler for logging comms events.
             */
            virtual void log_comms(comms_message_type severity, char const *message)
            {
               if(log != 0)
               {
                  Csi::OStrAscStream annotation;
                  Csi::LgrDate now(Csi::LgrDate::system());
                  now.format(annotation, "\"%Y-%m-%d %H:%M:%S%x\",\"");
                  switch(severity)
                  {
                  case comms_status:
                  case comms_status_neutral:
                     annotation << "S\",\"";
                     break;

                  case comms_warning:
                     annotation << "W\",\"";
                     break;

                  case comms_fault:
                     annotation << "F\",\"";
                     break;
                  }
                  annotation << message << "\"";
                  log->force_break(annotation.c_str());
               }
               if(severity == comms_warning || severity == comms_fault)
                  source->get_manager()->report_source_log(source, message);
            }

            /**
             * Overloads the base class method to add an annotation to the log.
             */
            virtual void log_debug(char const *object_name, char const *message)
            {
               if(log != 0)
               {
                  Csi::OStrAscStream annotation;
                  Csi::LgrDate now(Csi::LgrDate::system());
                  now.format(annotation, "\"%Y-%m-%d %H:%M:%S%x\",\"");
                  annotation << object_name << "\",\"" << message << "\"";
                  log->force_break(annotation.c_str());
               }
            }
         };


         void SourceRouter::onOneShotFired(uint4 id)
         {
            if(id == search_id)
            {
               Csi::PakBus::SerialPacketBase *port(static_cast<Csi::PakBus::SerialPacketBase *>(ports.front()));
               if(port->get_is_dialed())
               {
                  Csi::PakBus::SerialPacket hello_request(Csi::PakBus::SerialPacket::max_header_len);
                  hello_request.set_expect_more(Csi::PakBus::ExpectMoreCodes::neutral);
                  hello_request.set_destination(broadcast_address);
                  hello_request.set_source(this_node_address);
                  hello_request.addByte(Csi::PakBus::PakCtrl::Messages::hello_request_cmd);
                  hello_request.addByte(0);
                  port->send_serial_packet(
                     hello_request, broadcast_address, Csi::PakBus::SerialPacket::link_state_off_line);
                  search_id = 0;
                  search_id = timer->arm(this, 5000);
               }
               else
               {
                  port->on_needs_to_dial(Csi::PakBus::Priorities::extra_high);
                  search_id = timer->arm(this, 500);
               }
            }
            else
               Router::onOneShotFired(id);
         } // onOneShotFired


         void SourceRouter::start_search()
         {
            if(search_id == 0)
               search_id = timer->arm(this, 100);
         } // start_search


         void SourceRouter::end_search()
         {
            if(search_id != 0)
               timer->disarm(search_id);
         } // end_search


         void SourceRouter::on_link_change(
            uint2 node1, uint2 node2, Csi::PakBus::HopMetric hop_metric, link_change_type change)
         {
            Router::on_link_change(node1, node2, hop_metric, change);
            if(search_id != 0 && change == link_change_added)
            {
               uint2 neighbour_address;
               if(node1 == this_node_address)
                  neighbour_address = node2;
               else if(node2 == this_node_address)
                  neighbour_address = node1;
               if(source->pakbus_encryption_key.length())
                  set_cipher(neighbour_address, new Csi::PakBus::AesCipher(source->pakbus_encryption_key));
               source->set_pakbus_address(neighbour_address);
               timer->disarm(search_id);
            }
         } // on_link_change


         /**
          * Defines a base class for transactions posted for source operations.
          */
         class SourceTran: public Csi::PakBus::PakBusTran
         {
         protected:
            /**
             * Specifies the operation that owns this transaction.
             */
            SourceOperation *operation;
            
         public:
            /**
             * Constructor
             *
             * @param operation_ Specifies the operation that owns this transaction.
             */
            SourceTran(SourceOperation *operation_);

            /**
             * Destructor
             */
            virtual ~SourceTran();

            /**
             * Overloads the base class version to forward the failure to the operation.
             */
            virtual void on_failure(failure_type failure);

            /**
             * Overloads the base class version to handle an incoming PakBus message.
             */
            virtual void on_pakctrl_message(pakctrl_message_handle &message);

            /**
             * Overloads the base class version to handle an incoming BMP5 message.
             */
            virtual void on_bmp5_message(bmp5_message_handle &message);

            /**
             * Overloads the base class version to request a start from the operation.
             */
            virtual void start();

            /**
             * Overloads the base class version to report that focus has been set.
             */
            virtual void on_focus_start();

            /**
             * Overloads the base class version to write the operation description to the specified
             * stream.
             */
            virtual void get_transaction_description(std::ostream &desc);
         };

         
         /**
          * Defines a base class that manages PakBus transactions for the BMP5 data source.
          */
         class SourceOperation: public Csi::InstanceValidator
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this operation.
             */
            SourceOperation(Bmp5Source *source_);

            /**
             * Destructor
             */
            virtual ~SourceOperation()
            { close_transaction(); }

            /**
             * Handles a failure reported by the specified transaction.
             *
             * @param sender Specifies the transaction reporting the failure.
             *
             * @param failure Specifies the type of failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure) = 0;

            /**
             * Handles a received PakCtrl message for the specified transaction.
             *
             * @param sender Specifies the transaction calling this method.
             *
             * @param message Specifies the message that has been received.
             */
            typedef SourceTran::pakctrl_message_handle pakctrl_message_handle;
            virtual void on_pakctrl_message(
               SourceTran *sender, pakctrl_message_handle &message)
            { }

            /**
             * Handles a received BMP5 message for the specified transaction.
             *
             * @param sender Specifies the transaction for which the message was received.
             *
             * @param message Specifies the message that was received.
             */
            typedef SourceTran::bmp5_message_handle bmp5_message_handle;
            virtual void on_bmp5_message(
               SourceTran *sender, bmp5_message_handle &message)
            { }

            /**
             * Handles the event where focus has been set for a transaction owned by this
             * operation.
             *
             * @param sender Specifies the transaction for which focus has been set.
             */
            virtual void on_focus_start(SourceTran *sender) = 0;

            /**
             * Must be overloaded to write the description for the specified transaction.
             */
            virtual void get_transaction_description(
               SourceTran *sender, std::ostream &desc) = 0;

            /**
             * Can be overloaded to handle the event that this operation has been added.  By
             * default, this version will create the transaction and request its focus.
             */
            virtual void start()
            {
               transaction.bind(new SourceTran(this));
               source->get_router()->open_transaction(transaction.get_handle());
               transaction->set_time_out(10000);
            }

            /**
             * @return Can be overloaded to return true to indicate that this operation supports
             * data requests.
             */
            virtual bool supports_data_requests() const
            { return false; }

            /**
             * @return Can be overloaded to indicate that this is a terminal operation.
             */
            virtual bool supports_terminal() const
            { return false; }

            /**
             * Closes the transaction owned by this operation.
             */
            void close_transaction();

            /**
             * Assigns the transaction owned by this operation a new identifier.
             */
            void reassign_transaction();
            
         protected:
            /**
             * Specifies the source that owns this operation.
             */
            Bmp5Source *source;
            
            /**
             * Specifies the transaction used by this operation.
             */
            Csi::PolySharedPtr<Csi::PakBus::PakBusTran, SourceTran> transaction;

            friend class SourceTran;
            friend class Bmp5Source;
         };


         SourceOperation::SourceOperation(Bmp5Source *source_):
            source(source_)
         { }


         void SourceOperation::close_transaction()
         {
            if(transaction != 0 && source->get_router() != 0)
            {
               source->get_router()->close_transaction(
                  transaction->get_destination_address(), transaction->get_transaction_id());
               transaction.clear();
            }
         } // close_transaction


         void SourceOperation::reassign_transaction()
         {
            if(transaction != 0 && source->get_router() != 0)
            {
               source->get_router()->reassign_transaction_id(
                  transaction->get_destination_address(), transaction->get_transaction_id());
            }
         } // reassign_transaction
         

         SourceTran::SourceTran(SourceOperation *operation_):
            PakBusTran(
               operation_->source->get_router().get_rep(),
               operation_->source->get_router()->get_timer(),
               Csi::PakBus::Priorities::high,
               operation_->source->get_pakbus_address()),
            operation(operation_)
         { }


         SourceTran::~SourceTran()
         {
            operation = 0;
         } // destructor
         

         void SourceTran::on_failure(failure_type failure)
         {
            if(SourceOperation::is_valid_instance(operation))
               operation->on_failure(this, failure);
         } // on_failure


         void SourceTran::on_pakctrl_message(pakctrl_message_handle &message)
         {
            PakBusTran::on_pakctrl_message(message);
            if(SourceOperation::is_valid_instance(operation))
               operation->on_pakctrl_message(this, message);
         } // on_pakctrl_message


         void SourceTran::on_bmp5_message(bmp5_message_handle &message)
         {
            PakBusTran::on_bmp5_message(message);
            if(SourceOperation::is_valid_instance(operation))
               operation->on_bmp5_message(this, message);
         } // on_bmp5_message


         void SourceTran::start()
         {
            request_focus();
         } // start


         void SourceTran::on_focus_start()
         {
            if(SourceOperation::is_valid_instance(operation))
               operation->on_focus_start(this);
         } // on_focus_start


         void SourceTran::get_transaction_description(std::ostream &out)
         {
            if(SourceOperation::is_valid_instance(operation))
               operation->get_transaction_description(this, out);
         } // get_transaction_description


         /**
          * Defines a symbol that represents the BMP5 source.
          */
         class SourceSymbol: public SymbolBase
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this symbol.
             */
            SourceSymbol(Bmp5Source *source):
               SymbolBase(source, source->get_name())
            { }

            /**
             * @return Overloaded to return the symbol type.
             */
            symbol_type_code get_symbol_type() const
            { return type_bmp5_source; }

            /**
             * @return Returns true if the source is connected.
             */
            virtual bool is_connected() const
            { return source->is_connected(); }

            /**
             * @return Returns true if this symbol is enabled.
             */
            virtual bool is_enabled() const
            { return true; }

            /**
             * @return Returns true to indicate that this symbol can be expanded.
             */
            virtual bool can_expand() const
            { return true; }
         };


         /**
          * Defines an object that represents a piece of an array as reported in BMP5 table
          * definitions.
          */
         class Bmp5Piece
         {
         public:
            /**
             * Set to true if the fields are read-only.
             */
            bool const read_only;

            /**
             * Specifies the logger type code for this field.
             */
            CsiLgrTypeCode const data_type;
            
            /**
             * Specifies the name of this field.
             */
            StrUni const name;

            /**
             * Specifies the field processing details.
             */
            StrUni const process;

            /**
             * Specifies the field units.
             */
            StrUni const units;

            /**
             * Specifies the field description.
             */
            StrUni const description;

            /**
             * Specifies the begin index.
             */
            uint4 begin_index;

            /**
             * Specifies the number of array elements for this piece.
             */
            uint4 piece_size;

            /**
             * Specifies the array dimensions for this piece.
             */
            Csi::ArrayDimensions const dimensions;

            /**
             * Specifies the number for this piece.
             */
            uint2 const piece_no;

            /**
             * Constructor
             *
             * @param field_type Specifies the field type encoded in the least significant seven
             * bits and the read-only flag encoded in the most significant bit.
             *
             * @param field_name Specifies the name of the field.
             *
             * @param field_processing Specifies the processing string for the field.
             *
             * @param field_units Specifies the units for the field.
             *
             * @param field_description Specifies the description string for the field.
             *
             * @param begin_index_ Specifies the start linear offset for the piece in an array.
             *
             * @param piece_size_ Specifies the number of array elements in this piece.
             *
             * @param dimensions_ Specifies the dimensions of the array.
             *
             * @param piece_no_ Specifies the one-based number for this piece.
             */
            Bmp5Piece(
               byte field_type,
               StrUni const &field_name,
               StrUni const &field_process,
               StrUni const &field_units,
               StrUni const &field_description,
               uint4 begin_index_,
               uint4 piece_size_,
               Csi::ArrayDimensions const &dimensions_,
               uint2 const piece_no_):
               read_only((field_type & 0x80) != 0),
               data_type(static_cast<CsiLgrTypeCode>(field_type & 0x7f)),
               name(field_name),
               process(field_process),
               units(field_units),
               description(field_description),
               begin_index(begin_index_),
               piece_size(piece_size_),
               dimensions(dimensions_),
               piece_no(piece_no_)
            { }

            /**
             * @return Returns true if this piece represents a scalar.
             */
            bool is_scalar() const
            {
               bool rtn(dimensions.empty());
               if(data_type == LgrAscii && dimensions.size() == 1)
                  rtn = true;
               return rtn;
            }

            /**
             * @return Returns the formatted field name for a scalar.
             *
             * @param offset Specifies the linear index of the scalar.
             */
            StrUni format_value_name(uint4 offset) const
            {
               Csi::OStrUniStream rtn;
               rtn << name;
               if(!is_scalar())
               {
                  typedef std::vector<uint4> index_type;
                  index_type index(dimensions.size());
                  dimensions.to_index(index.begin(), offset);
                  rtn << L"(";;
                  for(index_type::iterator ii = index.begin(); ii != index.end(); ++ii)
                  {
                     if(ii != index.begin())
                        rtn << ",";
                     rtn << *ii;
                     if(data_type == LgrAscii && ii + 2 == index.end())
                        break;
                  }
                  rtn << L")";
               }
               return rtn.str();
            }
         };


         /**
          * Defines a base class that represents a scalar or an array symbol in a BMP5 datalogger
          * table.
          */
         class FieldSymbol: public SymbolBase
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this symbol.
             *
             * @param name Specifies the name of this symbol.
             *
             * @param piece_ Specifies the piece associated with this symbol.
             *
             * @param parent Specifies the parent to this symbol.
             */
            typedef Csi::SharedPtr<Bmp5Piece> piece_handle;
            FieldSymbol(
               Bmp5Source *source, StrUni const &name, piece_handle piece_, SymbolBase *parent):
               SymbolBase(source, name, parent),
               piece(piece_)
            { }

            /**
             * Destructor
             */
            virtual ~FieldSymbol()
            { piece.clear(); }

            /**
             * @return Returns true to indicate that this symbol has a data type.
             */
            virtual bool has_data_type() const
            { return true; }

            /**
             * @return Returns the CSI type code for the data type.
             */
            virtual CsiDbTypeCode get_data_type() const
            { return lgr_to_csi_type(piece->data_type); }

            /**
             * @return Returns true to indicate that this symbol has units.
             */
            virtual bool has_units() const
            { return true; }

            /**
             * @return Returns the units for the piece.
             */
            virtual StrUni get_units() const
            { return piece->units; }

            /**
             * @return Returns true to indicate that this symbol has a process string.
             */
            virtual bool has_process() const
            { return true; }

            /**
             * @return Returns the process string.
             */
            virtual StrUni get_process() const
            { return piece->process; }

            /**
             * @return Returns true to indicate that this symbol has a description.
             */
            virtual bool has_description() const
            { return true; }

            /**
             * @return Returns the description.
             */
            virtual StrUni get_description() const
            { return piece->description; }

            /**
             * @return Returns a reference to the piece.
             */
            piece_handle get_piece() const
            { return piece; }

         protected:
            /**
             * Specifies information about the field.
             */
            piece_handle piece;
         };


         /**
          * Defines a symbol that represents a scalar value in a BMP5 datalogger table.
          */
         class ScalarSymbol: public FieldSymbol
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies source that owns this symbol.
             *
             * @param piece Specifies the piece description.
             *
             * @param offset Specifies the linear offset for this value.
             *
             * @param parent Specifies the parent symbol.
             */
            ScalarSymbol(Bmp5Source *source, piece_handle piece, uint4 offset, SymbolBase *parent):
               FieldSymbol(source, piece->format_value_name(offset), piece, parent)
            { }

            /**
             * Destructor
             */
            virtual ~ScalarSymbol()
            { piece.clear(); }

            /**
             * @return Returns the symbol type.
             */
            virtual symbol_type_code get_symbol_type() const
            {  return type_scalar; }

            /**
             * @return Returns false to indicate that no further expansion is possible.
             */
            virtual bool can_expand() const
            { return false; }

            /**
             * Overloaded to format the uri for this symbol type.
             */
            virtual void format_uri(std::ostream &out) const
            {
               // if the parent symbol is an array, we will skip over that symbol so we can
               // substitute our own name.
               if(parent->get_symbol_type() == type_array)
               {
                  SymbolBase const *avo(parent->get_parent());
                  avo->format_uri(out);
                  out << "." << get_name();
               }
               else
                  SymbolBase::format_uri(out);
            }
            virtual void format_uri(std::wostream &out) const
            {
               // if the parent symbol is an array, we will skip over that symbol so we can
               // substitute our own name.
               if(parent->get_symbol_type() == type_array)
               {
                  SymbolBase const *avo(parent->get_parent());
                  avo->format_uri(out);
                  out << L"." << get_name();
               }
               else
                  SymbolBase::format_uri(out);
            }
         };
         
         
         /**
          * Defines a symbol that represents an array from a BMP5 datalogger table.
          */
         class ArraySymbol: public FieldSymbol
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this symbol.
             *
             * @param piece Specifies the piece structure that describes this array.
             *
             * @param parent Specifies the parent for this symbol.
             */
            ArraySymbol(Bmp5Source *source, piece_handle piece, SymbolBase *parent):
               FieldSymbol(source, piece->name, piece, parent)
            {
               // we need to add the set of scalar symbols for this array.
               uint4 increment(1);
               uint4 count(piece->piece_size);
               
               if(piece->data_type == LgrAscii)
               {
                  increment = piece->dimensions.back();
                  count /= increment;
               }
               for(uint4 i = 0; i < count; ++i)
               {
                  Csi::PolySharedPtr<SymbolBase, ScalarSymbol> scalar(
                     new ScalarSymbol(source, piece, piece->begin_index + i * increment, this));
                  push_back(scalar.get_handle());
               }
            }

            /**
             * @return Returns the symbol type.
             */
            virtual symbol_type_code get_symbol_type() const
            { return type_array; }

            /**
             * @return Overloaded to return true to indicate that one further level of expansion is
             * available.
             */
            virtual bool can_expand() const
            { return true; }
         };
         

         /**
          * Defines a symbol that represents a BMP5 datalogger table.
          */
         class TableSymbol: public SymbolBase
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this symbol.
             *
             * @param name Specifies the name of this table.
             *
             * @param table_no_ Specifies the table number.
             */
            TableSymbol(
               Bmp5Source *source, StrUni const &name, uint2 table_no_):
               SymbolBase(source, name, source->get_source_symbol().get_rep()),
               table_no(table_no_)
            { }

            /**
             * @return Overloaded to return the code for a table symbol.
             */
            virtual symbol_type_code get_symbol_type() const
            { return type_table; }

            /**
             * @return Overloaded to return true.
             */
            virtual bool is_enabled() const
            { return true; }

            /**
             * @return Overloaded to indicate that the table can be expanded,.
             */
            virtual bool can_expand() const
            { return true; }

            /**
             * @return Returns the assigned table number for this table.
             */
            uint2 get_table_no() const
            { return table_no; }

            /**
             * @return Returns the table definition signature.
             */
            uint2 get_table_sig() const
            { return table_sig; }

            /**
             * @param value Specifies the table definition signature.
             */
            void set_table_sig(uint2 value)
            { table_sig = value; }

            /**
             * @return Returns the allocated size of the table.
             */
            uint4 get_table_size() const
            { return table_size; }

            /**
             * @param value Specifies the allocated number of records for the table.
             */
            void set_table_size(uint4 value)
            { table_size = value; }

            /**
             * @return Returns the time offset for this table in nanoseconds.
             */
            int8 get_time_offset() const
            { return time_offset; }

            /**
             * @param value Specifies the time offset for this table in nanoseconds.
             */
            void set_time_offset(int8 value)
            { time_offset = value; }

            /**
             * @return Returns the time interval for this table in nanoseconds.
             */
            int8 get_interval() const
            { return interval; }

            /**
             * @param value Specifies the time interval for this table in nanoseconds.
             */
            void set_interval(int8 value)
            { interval = value; }

            /**
             * @return Returns the expected data type for time stamps.
             */
            CsiLgrTypeCode get_time_type() const
            { return time_type; }

            /**
             * @param value Specifies the expected data type for record time stamps.
             */
            void set_time_type(CsiLgrTypeCode value)
            { time_type = value; }

            /**
             * Adds a collection of fields based upon the specified piece parameters.
             *
             * @param piece Specifies the piece to add to the table.
             */
            typedef Csi::SharedPtr<Bmp5Piece> piece_handle;
            void add_piece(piece_handle piece);
            
         private:
            /**
             * Specifies the table number.
             */
            uint2 table_no;

            /**
             * Specifies the table definition signature.
             */
            uint2 table_sig;

            /**
             * Specifies the number of records allocated for this table.
             */
            uint4 table_size;

            /**
             * Specifies the data type used for time stamps for this table.
             */
            CsiLgrTypeCode time_type;

            /**
             * Specifies the time offset for this table in units of nanoseconds.
             */
            int8 time_offset;

            /**
             * Specifies the interval for this table in units if nanoseconds.
             */
            int8 interval;
         };


         void TableSymbol::add_piece(piece_handle piece)
         {
            if(piece->is_scalar())
               push_back(new ScalarSymbol(static_cast<Bmp5Source *>(source), piece, piece->begin_index, this));
            else
               push_back(new ArraySymbol(static_cast<Bmp5Source *>(source), piece, this));
         } // add_piece
         
         
         /**
          * Defines an operation that gets new table definitions from the datalogger.
          */
         class OpGetTableDefs: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that requested this operation.
             */
            OpGetTableDefs(Bmp5Source *source_);

            /**
             * Overloads the base class version to handle the event where focus has been granted.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to handle a transaction failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to handle an incoming BMP5 message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to describe this operation.
             */
            virtual void get_transaction_description(
               SourceTran *sender, std::ostream &desc);

         private:
            /**
             * Handles the response to the get compile results command.
             */
            void on_get_compile_results_ack(bmp5_message_handle &message);

            /**
             * Sends the next request for a tdf fragment.
             */
            void send_next_tdf_request();

            /**
             * Handles the file receive response for reading the .TDF file.
             */
            void on_read_file_ack(bmp5_message_handle &message);

         private:
            /**
             * Records the state of this operation.
             */
            enum state_type
            {
               state_get_compile_results,
               state_get_table_defs
            } state;

            /**
             * Specifies the table definitions file content.
             */
            Csi::PakBus::Bmp5Message tdf_content;

            /**
             * Specifies the last swath that was specified.
             */
            uint2 last_swath;

            /**
             * Used to buffer received fragments.
             */
            StrBin frag_buff;
         };


         OpGetTableDefs::OpGetTableDefs(Bmp5Source *source_):
            SourceOperation(source_),
            state(state_get_compile_results)
         { }


         void OpGetTableDefs::on_focus_start(SourceTran *sender)
         {
            if(state == state_get_compile_results)
            {
               SourceTran::bmp5_message_handle command(new Csi::PakBus::Bmp5Message);
               command->addUInt2(source->get_security_code());
               command->set_message_type(Csi::PakBus::Bmp5Messages::get_compile_results_cmd);
               sender->set_time_out(10000);
               sender->send_bmp5_message(command);
            }
            else
               send_next_tdf_request();
         } // on_focus_start


         void OpGetTableDefs::on_failure(SourceTran *sender, SourceTran::failure_type failure)
         {
            if(source->connect_state != Bmp5Source::connect_state_connected)
            {
               StrAsc reason;
               switch(failure)
               {
               case Csi::PakBus::PakCtrl::DeliveryFailure::unreachable_destination:
                  reason = "unreachable";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::unreachable_high_level_protocol:
               case Csi::PakBus::PakCtrl::DeliveryFailure::unsupported_message_type:
                  reason = "unsupported";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::timed_out_or_resource_error:
               case Csi::PakBus::PakCtrl::DeliveryFailure::malformed_message:
                  reason = "timed out or communication failure";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::failed_static_route:
                  reason = "routing failed";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::packet_too_big_64:
               case Csi::PakBus::PakCtrl::DeliveryFailure::packet_too_big_90:
               case Csi::PakBus::PakCtrl::DeliveryFailure::packet_too_big_128:
               case Csi::PakBus::PakCtrl::DeliveryFailure::packet_too_big_256:
               case Csi::PakBus::PakCtrl::DeliveryFailure::packet_too_big_512:
                  reason = "packet is too large";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::unsupported_encryption_cipher:
                  reason = "unsupported encryption cipher";
                  break;
                  
               case Csi::PakBus::PakCtrl::DeliveryFailure::encryption_required:
                  reason = "encryption is required";
                  break;
                  
               default:
                  reason = "unknown failure";
                  break;
               }
               source->on_connect_failure(reason);
            }
         } // on_failure


         void OpGetTableDefs::on_bmp5_message(SourceTran *sender, SourceTran::bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::please_wait_notification)
            {
               byte message_type(message->readByte());
               uint4 estimate_sec(message->readUInt2());
               sender->set_time_out(estimate_sec * 1000);
            }
            else
            {
               if(state == state_get_compile_results)
                  on_get_compile_results_ack(message);
               else
                  on_read_file_ack(message);
            }
         } // on_bmp5_message


         void OpGetTableDefs::get_transaction_description(SourceTran *sender, std::ostream &out)
         {
            if(state == state_get_compile_results)
               out << "get compile results";
            else
               out << "read table definitions";
         } // get_transaction_description


         void OpGetTableDefs::on_get_compile_results_ack(bmp5_message_handle &message)
         {
            byte response_code(message->readByte());
            if(response_code == 0)
            {
               // we need to parse the contents of the message
               message->readAsciiZ(source->os_version);
               source->os_sig = message->readUInt2();
               message->readAsciiZ(source->serial_no);
               message->readAsciiZ(source->power_up_program);
               source->compile_state = (Bmp5Source::compile_state_type)message->readByte();
               message->readAsciiZ(source->program_name);
               source->program_sig = message->readUInt2();
               source->compile_time = message->readNSec();
               message->readAsciiZ(source->compile_result);
               if(message->whatsLeft())
               {
                  message->readAsciiZ(source->model_no);
                  message->readAsciiZ(source->station_name);
               }

               // we can now switch state and start requesting table definitions.
               state = state_get_table_defs;
               reassign_transaction();
               send_next_tdf_request();
            }
            else
               source->on_connect_failure("permission denied");
         } // on_get_compile_results_ack


         void OpGetTableDefs::send_next_tdf_request()
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
            cmd->addUInt2(source->security_code);
            cmd->addAsciiZ(".TDF");
            cmd->addBool(false);
            cmd->addUInt4(tdf_content.whatsLeft());
            last_swath = (uint2)source->router->get_max_body_len(source->pakbus_address) - 5;
            cmd->addUInt2(last_swath);
            transaction->send_bmp5_message(cmd);
            transaction->set_time_out(10000);
         } // send_next_tdf_request


         void OpGetTableDefs::on_read_file_ack(bmp5_message_handle &message)
         {
            byte resp_code(message->readByte());
            if(resp_code == 0)
            {
               uint4 start_offset(message->readUInt4());
               if(start_offset == tdf_content.whatsLeft())
               {
                  frag_buff.cut(0);
                  message->readBytes(frag_buff, message->whatsLeft());
                  tdf_content.addBytes(frag_buff.getContents(), (uint4)frag_buff.length());
                  if(frag_buff.length() == last_swath)
                     send_next_tdf_request();
                  else
                  {
                     byte tdf_version(tdf_content.readByte());
                     StrAsc table_name;
                     uint2 table_no(0);
                     byte field_type;
                     StrAsc field_name;
                     StrAsc field_processing;
                     StrAsc field_units;
                     StrAsc field_description;
                     uint4 begin_index;
                     uint4 piece_size;
                     Csi::ArrayDimensions dimensions;
                     uint4 dimension;
                     
                     if(tdf_version == 1)
                     {
                        // we will announce that all the old tables are being deleted and remove
                        // them from the source symbol.  We could be fancier about this by trying to
                        // determine whether a table has really changed but there is little reason
                        // for doing so now.
                        for(SymbolBase::iterator ti = source->source_symbol->begin(); ti != source->source_symbol->end(); ++ti)
                           source->source_symbol->get_browser()->send_symbol_removed(*ti, SymbolBrowserClient::remove_table_deleted);
                        source->source_symbol->children.clear();

                        // we can now attempt to process all of the table definitions in the file.
                        while(tdf_content.whatsLeft() > 0)
                        {
                           uint4 table_start(tdf_content.getReadIdx());
                           Csi::PolySharedPtr<SymbolBase, TableSymbol> table;
                           uint2 piece_no(0);
                           
                           tdf_content.readAsciiZ(table_name);
                           table.bind(new TableSymbol(source, table_name, ++table_no));
                           table->set_table_size(tdf_content.readUInt4());
                           table->set_time_type((CsiLgrTypeCode)tdf_content.readByte());
                           table->set_time_offset(tdf_content.readNSec().get_nanoSec());
                           table->set_interval(tdf_content.readNSec().get_nanoSec());
                           while(tdf_content.whatsLeft() >= 1)
                           {
                              field_type = tdf_content.readByte();
                              if(field_type != 0)
                              {
                                 tdf_content.readAsciiZ(field_name);
                                 tdf_content.movePast(1); // skip reserved byte
                                 tdf_content.readAsciiZ(field_processing);
                                 tdf_content.readAsciiZ(field_units);
                                 tdf_content.readAsciiZ(field_description);
                                 begin_index = tdf_content.readUInt4();
                                 piece_size = tdf_content.readUInt4();
                                 dimensions.clear();
                                 while((dimension = tdf_content.readUInt4()) != 0)
                                    dimensions.add_dimension(dimension);
                                 table->add_piece(
                                    new Bmp5Piece(
                                       field_type,
                                       field_name,
                                       field_processing,
                                       field_units,
                                       field_description,
                                       begin_index,
                                       piece_size,
                                       dimensions,
                                       ++piece_no));
                              }
                              else
                                 break;
                           }
                           table->set_table_sig(
                              Csi::calcSigFor(tdf_content.getMsg() + table_start, tdf_content.getReadIdx() - table_start));
                           source->source_symbol->push_back(table.get_handle());
                           source->source_symbol->get_browser()->send_symbol_added(table.get_handle());
                        }
                        source->on_connect_complete();
                        source->remove_operation(this);
                     }
                     else
                        source->on_connect_failure("unsupported table definitions");
                  }
               }
            }
            else
               source->on_connect_failure("permission failed");
         } // on_read_file_ack


         /**
          * Defines an operation object that handles one or more data requests.
          */
         class OpDataRequest: public SourceOperation, public Csi::EventReceiver
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this operation.
             *
             * @param request Specifies the first request for this operation.
             */
            typedef Bmp5Source::request_handle request_handle;
            OpDataRequest(Bmp5Source *source_, request_handle &request);

            /**
             * Destructor
             */
            virtual ~OpDataRequest();

            /**
             * Overloads the base class version so that the transaction is not immediately
             * initialised.
             */
            virtual void start()
            { }

            /**
             * Adds a request to the set associated with this operation provided that the request is
             * compatible with the first request and that this operation is not already started.
             *
             * @return Returns true if the request was accepted.
             *
             * @param request Specifies the request to add.
             */
            bool add_request(request_handle &request);

            /**
             * Removes the specified request from the set serviced by this operation.
             *
             * @return Returns true if the request was found and removed.
             */
            bool remove_request(request_handle &request);

            /**
             * @return Returns the number fof requests serviced by this operation.
             */
            uint4 get_requests_count() const
            { return (uint4)requests.size(); }

            /**
             * Generates the record description, if needed, and starts the datalogger transaction.
             */
            void poll();

            /**
             * Handles a failure reported by the datalogger transaction.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Handles an incoming BMP5 message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Handles the set focus event.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to describe the transaction.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);

            /**
             * @return Overloads the base class version to indicate that this operation will poll
             * for data.
             */
            virtual bool supports_data_requests() const
            { return true; }
            
            /**
             * Overloads the base class version to handle events associated with polling.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
            /**
             * Generates a record description that will satisfy all requests associated with this
             * operation.
             *
             * @return Returns SinkBase::sink_failure_unknown if all of the requests could be started.
             */
            SinkBase::sink_failure_type generate_record_desc();

            /**
             * Generates and sends the next command message for data collection.
             */
            void send_next();

            /**
             * Reads a collection of records from the specified message.
             */
            typedef Csi::SharedPtr<Broker::Record> record_handle;
            typedef std::deque<record_handle> cache_type;
            void read_records(
               cache_type &records,
               Csi::PakBus::Bmp5Message &message,
               uint4 begin_record_no,
               uint4 records_count);

            /**
             * Handles records received when asking for the newest record.
             */
            void on_get_newest(cache_type &records);

            /**
             * Handles records received when asking for a date range.
             */
            void on_date_range(cache_type &records);

            /**
             * Handles records received while collecting holes.
             */
            void on_collect_holes(cache_type &records);

            /**
             * @return Returns the calculated block size for the specified number of records.  This
             * calculation takes into account the size and presence of time stamps.
             *
             * @param records_count Specifies the number of records anticipated in the block.
             */
            uint4 get_native_block_size(uint4 records_count);
            
         private:
            /**
             * Specifies the collection of requests that this operation will service.
             */
            typedef SinkBase::requests_type requests_type;
            requests_type requests;

            /**
             * Specifies the record description that will be used to satisfy all of the requests.
             */
            typedef Broker::RecordDesc record_desc_type;
            Csi::SharedPtr<record_desc_type> record_desc;

            /**
             * Specifies a collection of records that can be used to report new data.
             */
            cache_type cache;

            /**
             * Specifies the table area that will be polled by this cursor.
             */
            Csi::PolySharedPtr<SymbolBase, TableSymbol> table_symbol;

            /**
             * Specifies the collection of pieces that are selected.
             */
            typedef Csi::SharedPtr<Bmp5Piece> piece_handle;
            typedef std::deque<piece_handle> pieces_type;
            pieces_type pieces;

            /**
             * Specifies the state of this request.
             */
            enum state_type
            {
               state_not_started,
               state_get_newest,
               state_date_range,
               state_collect_holes,
               state_satisfied
            } state;

            /**
             * Holds the newest record
             */
            record_handle newest_record;

            /**
             * Specifies the range of record numbers that have already been collected.
             */
            Csi::RangeList collected_records;

            /**
             * Specifies the range of records that are expected in the current poll.
             */
            Csi::RangeList expected_records;

            /**
             * Specifies a buffer used to read records.
             */
            StrBin record_buff;

            /**
             * Specifies a message buffer used to buffer record data as it is read from the
             * datalogger.  This will be used for both partial as well as for complete record
             * blocks.
             */
            bmp5_message_handle record_data;
         };


         OpDataRequest::OpDataRequest(Bmp5Source *source, request_handle &request):
            SourceOperation(source),
            state(state_not_started),
            record_data(new Csi::PakBus::Bmp5Message)
         { requests.push_back(request); }


         OpDataRequest::~OpDataRequest()
         {
            requests.clear();
            record_data.clear();
         } // destructor


         bool OpDataRequest::add_request(request_handle &request)
         {
            bool rtn(false);
            if(!requests.empty() && record_desc == 0)
            {
               try
               {
                  request_handle &first(requests.front());
                  Csi::PolySharedPtr<WartBase, TableFieldUri> first_wart(first->get_wart());
                  Csi::PolySharedPtr<WartBase, TableFieldUri> request_wart(request->get_wart());
                  if(first_wart->get_table_name() == request_wart->get_table_name() && first->is_compatible(*request))
                  {
                     requests.push_back(request);
                     rtn = true;
                  }
               }
               catch(std::exception &)
               { rtn = false; }
            }
            return rtn;
         } // add_request


         bool OpDataRequest::remove_request(request_handle &request)
         {
            bool rtn(false);
            requests_type::iterator ri(std::find(requests.begin(), requests.end(), request));
            if(ri != requests.end())
            {
               rtn = true;
               requests.erase(ri);
               if(requests.empty())
               {
                  // @todo: we need to release the current transaction
               }
            }
            return rtn;
         } // remove_request


         void OpDataRequest::poll()
         {
            // if this is the first time being polled, the record description will be empty and we
            // will need to generate it.
            if(record_desc == 0)
            {
               SinkBase::sink_failure_type init_state(generate_record_desc());
               if(init_state != SinkBase::sink_failure_unknown)
               {
                  // @todo: we need to report a general failure.
                  return;
               }
            }

            // if polling is already underway, the transaction will be active.
            if(transaction == 0)
            {
               // we will initialise the transaction and allow it to request focus once it has been
               // initialised.
               state = state_not_started;
               transaction.bind(new SourceTran(this));
               source->get_router()->open_transaction(transaction.get_handle());
               transaction->set_time_out(10000);
            }
         } // poll


         void OpDataRequest::on_failure(
            SourceTran *sender, SourceTran::failure_type failure)
         {
            state = state_not_started;
            close_transaction();
         } // on_failure


         uint4 OpDataRequest::get_native_block_size(uint4 records_count)
         {
            record_handle record;
            uint4 rtn;
            uint4 time_stamp_size(0);

            switch(table_symbol->get_time_type())
            {
            case LgrSec:
               time_stamp_size = 4;
               break;
               
            case LgrNSec:
            case LgrNSecLsf:
               time_stamp_size = 8;
               break;

            case LgrUSec:
               time_stamp_size = 6;
               break;
            }
            if(!cache.empty())
               record = cache.front();
            else
            {
               record.bind(new Broker::Record(record_desc, *source->get_manager()->get_value_factory()));
               cache.push_back(record);
            }
            rtn = records_count * record->get_value_buffer_len();
            if(table_symbol->get_interval() == 0)
               rtn += time_stamp_size * records_count;
            else
               rtn += time_stamp_size;
            return rtn;
         } // get_native_block_size


         void OpDataRequest::on_bmp5_message(
            SourceTran *sender, bmp5_message_handle &message)
         {
            sender->clear_time_out();
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::collect_data_ack)
            {
               byte rcd(message->readByte());
               if(rcd == 0)
               {
                  // we need to read the records
                  cache_type records;
                  bool completing_partial(false);
                  while(message->whatsLeft() > sizeof(uint2))
                  {
                     // read the fragment header
                     uint2 table_no(message->readUInt2());
                     uint4 begin_record_no(message->readUInt4());
                     uint2 number_of_records(message->readUInt2());
                     bool partial_record((number_of_records & 0x8000) != 0);

                     if(partial_record)
                     {
                        // we need to ignore the two byte offset least significant two bytes that
                        // will follow.  Also we will assume that only one partial record can ever
                        // arrive in one fragment.
                        message->readUInt2();
                        number_of_records = 1;
                     }
                     else
                        number_of_records &= 0x7fff;

                     // what happens next depends upon whether there is enough data accumulated in
                     // the fragment to satisfy at least one record
                     uint4 required_size(get_native_block_size(number_of_records));
                     uint4 block_size(Csi::csimin(required_size, message->whatsLeft() - 1));
                     if(number_of_records == 0)
                        continue;
                     record_data->addBytes(message->objAtReadIdx(), block_size);
                     message->movePast(block_size);
                     if(partial_record && record_data->whatsLeft() < required_size)
                     {
                        bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                        cmd->set_message_type(Csi::PakBus::Bmp5Messages::collect_data_cmd);
                        cmd->addUInt2(source->security_code);
                        cmd->addByte(8); // collect partial record
                        cmd->addUInt2(table_symbol->get_table_no());
                        cmd->addUInt2(table_symbol->get_table_sig());
                        cmd->addUInt4(begin_record_no);
                        cmd->addUInt4(record_data->length() - record_data->get_headerLen());
                        if(pieces.size() != table_symbol->size())
                        {
                           for(pieces_type::const_iterator pi = pieces.begin(); pi != pieces.end(); ++pi)
                              cmd->addUInt2((*pi)->piece_no);
                        }
                        cmd->addUInt2(0);
                        completing_partial = true;
                        reassign_transaction();
                        transaction->send_bmp5_message(cmd);
                     }
                     else if(number_of_records > 0)
                     {
                        read_records(records, *record_data, begin_record_no, number_of_records);
                        record_data->clear();
                     }
                  }

                  // if we are not waiting for a partial, we can now process the collected records
                  if(!completing_partial)
                  {
                     if(state == state_get_newest)
                        on_get_newest(records);
                     else if(state == state_date_range)
                        on_date_range(records);
                     else if(state == state_collect_holes)
                        on_collect_holes(records);
                  }
               }
               else
               {
                  SinkBase::sink_failure_type failure(SinkBase::sink_failure_unknown);
                  requests_type temp(requests);
                  switch(rcd)
                  {
                  case 1:
                     failure = SinkBase::sink_failure_server_security;
                     break;

                  case 2:
                     failure = SinkBase::sink_failure_connection_failed;
                     break;

                  case 7:
                     failure = SinkBase::sink_failure_invalid_table_name;
                     break;
                  }
                  source->set_retry_requests();
                  for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
                  {
                     request_handle &request(*ri);
                     request->set_state(source, Request::state_error);
                     if(SinkBase::is_valid_instance(request->get_sink()))
                        request->get_sink()->on_sink_failure(source->get_manager(), request, failure);
                  }
                  source->remove_operation(this);
               }
            }
            else if(message->get_message_type() == Csi::PakBus::Bmp5Messages::please_wait_notification)
            {
               byte message_type(message->readByte());
               uint4 estimate_sec(message->readUInt2());
               sender->set_time_out(estimate_sec * 1000);
            }
         } // on_bmp5_message


         void OpDataRequest::on_focus_start(
            SourceTran *sender)
         {
            send_next();
         } // on_focus_start


         void OpDataRequest::get_transaction_description(
            SourceTran *sender, std::ostream &desc)
         {
            switch(state)
            {
            case state_not_started:
            case state_get_newest:
               desc << "get newest record";
               break;

            case state_date_range:
               desc << "date range query";
               break;

            case state_collect_holes:
               desc << "collect holes";
               break;
            }
         } // get_transaction_description


         void OpDataRequest::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
         } // receive


         SinkBase::sink_failure_type OpDataRequest::generate_record_desc()
         {
            SinkBase::sink_failure_type rtn(SinkBase::sink_failure_unknown);
            if(!requests.empty())
            {
               // we first need to find the table symbol associated with the requests
               Csi::SharedPtr<SymbolBase> source_symbol(source->get_source_symbol());
               request_handle &first(requests.front());
               Csi::PolySharedPtr<WartBase, TableFieldUri> first_wart(first->get_wart());
               table_symbol.clear();
               for(SymbolBase::iterator si = source_symbol->begin(); si != source_symbol->end() && table_symbol == 0; ++si)
               {
                  Csi::SharedPtr<SymbolBase> &symbol(*si);
                  if(symbol->get_name() == first_wart->get_table_name())
                     table_symbol = symbol;
               }
               if(table_symbol != 0)
               {
                  // we need to select all of the pieces that match the column names given in the
                  // requests.
                  pieces.clear();
                  for(SymbolBase::iterator si = table_symbol->begin(); si != table_symbol->end(); ++si)
                  {
                     Csi::PolySharedPtr<SymbolBase, FieldSymbol> field_symbol(*si);
                     piece_handle field_piece(field_symbol->get_piece());
                     bool use_piece(false);

                     for(requests_type::iterator ri = requests.begin(); !use_piece && ri != requests.end(); ++ri)
                     {
                        request_handle &request(*ri);
                        Csi::PolySharedPtr<WartBase, TableFieldUri> wart(request->get_wart());
                        if(wart->get_column_name().length() == 0)
                           use_piece = true;
                        else
                        {
                           Broker::ValueName value_name(wart->get_column_name());
                           if(value_name.get_column_name() == field_piece->name)
                           {
                              uint4 offset = field_piece->dimensions.to_offset(value_name.begin(), value_name.end());
                              if(offset >= field_piece->begin_index && offset < field_piece->begin_index + field_piece->piece_size)
                                 use_piece = true;
                           }
                        }
                        if(use_piece && request->get_state() != Request::state_started)
                           request->set_state(source, Request::state_started);
                     }
                     if(use_piece)
                        pieces.push_back(field_piece);
                  }

                  // the list of selected pieces should already be sorted so all we have to do is to
                  // generate the record description.
                  Csi::OStrAscStream temp;
                  record_desc.bind(new record_desc_type(source->station_name, table_symbol->get_name()));
                  record_desc->model_name = source->model_no;
                  record_desc->serial_number = source->serial_no;
                  record_desc->os_version = source->os_version;
                  record_desc->dld_name = source->program_name;
                  temp.str("");
                  temp << source->program_sig;
                  record_desc->dld_signature = temp.str();
                  for(pieces_type::iterator pi = pieces.begin(); pi != pieces.end(); ++pi)
                  {
                     piece_handle &piece(*pi);
                     for(uint4 i = 0; i < piece->piece_size; ++i)
                     {
                        record_desc_type::value_type value_desc(new Cora::Broker::ValueDesc);
                        value_desc->name = piece->name;
                        value_desc->data_type = lgr_to_csi_type(piece->data_type);
                        value_desc->modifying_cmd = piece->read_only ? 0 : 1;
                        value_desc->units = piece->units;
                        value_desc->process = piece->process;
                        value_desc->description = piece->description;
                        value_desc->array_address.resize(piece->dimensions.size());
                        piece->dimensions.to_index(value_desc->array_address.begin(), piece->begin_index + i);
                        record_desc->values.push_back(value_desc);
                     }
                  }

                  // we can now generate a record using the new description
                  record_handle record(new Broker::Record(record_desc, *source->get_manager()->get_value_factory()));
                  cache.push_back(record);
                  for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                  {
                     request_handle &request(*ri);
                     if(request->get_state() == Request::state_started)
                     {
                        Csi::PolySharedPtr<WartBase, TableFieldUri> wart(request->get_wart());
                        request->set_value_indices(*record, wart->get_column_name());
                        request->get_sink()->on_sink_ready(source->get_manager(), request, record);
                     }
                     else
                     {
                        // @todo: we need to post an event to report a request failure
                     }
                  }
               }
               else
                  rtn = SinkBase::sink_failure_invalid_table_name;
            }
            return rtn;
         } // generate_record_desc


         void OpDataRequest::send_next()
         {
            if(!requests.empty())
            {
               bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
               request_handle &first(requests.front());
               cmd->set_message_type(Csi::PakBus::Bmp5Messages::collect_data_cmd);
               cmd->addUInt2(source->security_code);
               if(state == state_not_started)
               {
                  // the first thing that we need to do is to collect the newest record from the table.
                  cmd->addByte(5);
                  cmd->addUInt2(table_symbol->get_table_no());
                  cmd->addUInt2(table_symbol->get_table_sig());
                  cmd->addUInt4(1); // newest record
                  if(pieces.size() != table_symbol->size())
                  {
                     for(pieces_type::iterator pi = pieces.begin(); pi != pieces.end(); ++pi)
                     {
                        piece_handle &piece(*pi);
                        cmd->addUInt2(piece->piece_no);
                     }
                  }
                  cmd->addUInt2(0);
                  state = state_get_newest;
               }
               else if(state == state_date_range)
               {
                  cmd->addByte(7);
                  cmd->addUInt2(table_symbol->get_table_no());
                  cmd->addUInt2(table_symbol->get_table_sig());
                  if(first->get_start_option() == Request::start_relative_to_newest)
                  {
                     cmd->addNSec(newest_record->get_stamp() + first->get_backfill_interval());
                     cmd->addNSec(newest_record->get_stamp());
                  }
                  else
                  {
                     cmd->addNSec(first->get_start_time());
                     if(newest_record->get_stamp() < first->get_end_time())
                        cmd->addNSec(newest_record->get_stamp());
                     else
                        cmd->addNSec(first->get_end_time());
                  }
                  if(pieces.size() != table_symbol->size())
                  {
                     for(pieces_type::iterator pi = pieces.begin(); pi != pieces.end(); ++pi)
                     {
                        piece_handle &piece(*pi);
                        cmd->addUInt2(piece->piece_no);
                     }
                  }
                  cmd->addUInt2(0);
               }
               else if(state == state_collect_holes)
               {
                  cmd->addByte(6);
                  for(Csi::RangeList::const_iterator ri = expected_records.begin(); ri != expected_records.end(); ++ri)
                  {
                     cmd->addUInt2(table_symbol->get_table_no());
                     cmd->addUInt2(table_symbol->get_table_sig());
                     cmd->addUInt4(ri->first);
                     cmd->addUInt4(ri->second + 1);
                     if(pieces.size() != table_symbol->size())
                     {
                        for(pieces_type::iterator pi = pieces.begin(); pi != pieces.end(); ++pi)
                        {
                           piece_handle &piece(*pi);
                           cmd->addUInt2(piece->piece_no);
                        }
                     }
                     cmd->addUInt2(0);
                  }
               }
               if(cmd != 0)
                  transaction->send_bmp5_message(cmd);
            }
         } // send_next


         void OpDataRequest::read_records(
            cache_type &records, Csi::PakBus::Bmp5Message &message, uint4 begin_record_no, uint4 records_count)
         {
            Csi::LgrDate stamp;

            for(uint4 i = 0; i < records_count; ++i)
            {
               record_handle record;
               if(!cache.empty())
               {
                  record = cache.front();
                  cache.pop_front();
               }
               else
                  record.bind(new Broker::Record(record_desc, *source->get_manager()->get_value_factory()));
               if(i == 0 || table_symbol->get_interval() == 0)
               {
                  switch(table_symbol->get_time_type())
                  {
                  case LgrSec:
                     stamp = message.readUInt4() * Csi::LgrDate::nsecPerSec;
                     break;

                  case LgrNSec:
                     stamp = message.readNSec();
                     break;

                  case LgrNSecLsf:
                     stamp = message.readNSecLsf();
                     break;
                  }
               }
               else
                  stamp += table_symbol->get_interval();
               message.readBytes(record_buff, record->get_value_buffer_len());
               record->read(begin_record_no + i, 0, stamp, record_buff.getContents(), (uint4)record_buff.length());
               records.push_back(record);
            }
         } // read_records


         namespace
         {
            struct record_collected
            {
               Csi::RangeList &collected;
               record_collected(Csi::RangeList &collected_):
                  collected(collected_)
               { }

               bool operator ()(Csi::SharedPtr<Cora::Broker::Record> &record) const
               {
                  return collected.is_element(record->get_record_no());
               }
            };


            struct record_outside_time
            {
               Csi::LgrDate const start;
               Csi::LgrDate const stop;
               record_outside_time(Csi::LgrDate const &start_, Csi::LgrDate const &stop_):
                  start(start_),
                  stop(stop_)
               { }

               bool operator ()(Csi::SharedPtr<Cora::Broker::Record> &record) const
               { return record->get_stamp() < start || record->get_stamp() > stop; }
            };
         };


         void OpDataRequest::on_get_newest(cache_type &records)
         {
            // we will eliminate any records from the set that have already been collected.
            cache_type::iterator ri(
               std::remove_if(
                  records.begin(), records.end(), record_collected(collected_records)));
            if(ri != records.end())
            {
               std::copy(ri, records.end(), std::back_inserter(cache));
               records.erase(ri, records.end());
            }

            // we will now proceed only if there are records to process
            if(!records.empty() && !requests.empty())
            {
               // for real time queries, we are already done once the records have been sent
               request_handle &first(requests.front());
               if(first->get_order_option() == Request::order_real_time || table_symbol->get_table_size() == 1)
               {
                  std::copy(records.begin(), records.end(), std::back_inserter(cache));
                  collected_records.add_range(records.front()->get_record_no(), records.back()->get_record_no());
                  for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                  {
                     requests_type::value_type &request(*ri);
                     request->set_expect_more_data(false);
                  }
                  close_transaction();
                  SinkBase::report_sink_records(source->get_manager(), requests, records);
               }
               else
               {
                  // the next state will depend upon whether we have handled records previously
                  if(collected_records.empty())
                  {
                     // since we have not collected records previously, we will need to pay
                     // attention to the start option.
                     if(first->get_start_option() == Request::start_relative_to_newest ||
                        first->get_start_option() == Request::start_date_query)
                     {
                        newest_record = records.back();
                        state = state_date_range;
                     }
                     else if(first->get_start_option() == Request::start_at_record)
                     {
                        newest_record = records.back();
                        expected_records.clear();
                        expected_records.add_range(first->get_record_no(), records.back()->get_record_no() - 1);
                        state = state_collect_holes;
                     }
                     else if(first->get_start_option() == Request::start_at_offset_from_newest)
                     {
                        uint4 offset(Csi::csimin(table_symbol->get_table_size(), first->get_start_record_offset()));
                        expected_records.clear();
                        newest_record = records.back();
                        expected_records.add_range(
                           records.back()->get_record_no() - offset, records.back()->get_record_no() - 1);
                     }
                  }
                  else
                  {
                     // since we have already collected, we will start assuming that we are going to
                     // collect everything since last collection.  We will start with the whole
                     // table and then remove everything that we have collected.
                     Csi::RangeList everything;
                     if(records.back()->get_record_no() >= table_symbol->get_table_size())
                        everything.add_range(records.back()->get_record_no() - table_symbol->get_table_size(), records.front()->get_record_no() - 1);
                     else
                        everything.add_range(0, records.front()->get_record_no() - 1);
                     expected_records = everything - collected_records;
                     newest_record = records.back();
                     expected_records.remove_range(newest_record->get_record_no(), newest_record->get_record_no());
                     if(!expected_records.empty())
                        state = state_collect_holes;
                     else
                     {
                        std::copy(records.begin(), records.end(), std::back_inserter(cache));
                        for(requests_type::iterator rqi = requests.begin(); rqi != requests.end(); ++rqi)
                        {
                           request_handle &request(*rqi);
                           request->set_expect_more_data(false);
                        }
                        SinkBase::report_sink_records(source->get_manager(), requests, records);
                        collected_records.add_range(records.front()->get_record_no(), records.back()->get_record_no());
                        state = state_not_started;
                     }
                  }

                  // we will assign a new identifier to the transaction and will then
                  // release/request focus to allow us to share the link
                  if(state != state_not_started)
                  {
                     reassign_transaction();
                     transaction->request_focus();
                     transaction->release_focus();
                  }
                  else
                     close_transaction();
               }
            }
            else
               close_transaction();
         } // on_get_newest


         void OpDataRequest::on_date_range(cache_type &records)
         {
            // we will eliminate any records from the set that have already been collected.
            cache_type::iterator ri(
               std::remove_if(
                  records.begin(), records.end(), record_collected(collected_records)));
            if(ri != records.end())
            {
               std::copy(ri, records.end(), std::back_inserter(cache));
               records.erase(ri, records.end());
            }
            if(!requests.empty() && requests.front()->get_start_option() == Request::start_date_query)
            {
               request_handle &first(requests.front());
               ri = std::remove_if(
                  records.begin(), records.end(), record_outside_time(first->get_start_time(), first->get_end_time()));
               if(ri != records.end())
                  records.erase(ri, records.end());
            }

            // we can add all of the remaining records to the collected records list
            for(cache_type::iterator ri = records.begin(); ri != records.end(); ++ri)
            {
               record_handle &record(*ri);
               collected_records.add_range(record->get_record_no(), record->get_record_no());
            }

            // we now need to determine what records to collect as holes
            expected_records.clear();
            if(!records.empty() && records.back()->get_record_no() + 1 != newest_record->get_record_no())
               expected_records.add_range(records.back()->get_record_no() + 1, newest_record->get_record_no() - 1);
            if(expected_records.empty())
            {
               state = state_not_started;
               collected_records.clear();
               if(newest_record->get_record_no() >= table_symbol->get_table_size())
                  collected_records.add_range(newest_record->get_record_no() - table_symbol->get_table_size(), newest_record->get_record_no());
               else
                  collected_records.add_range(0, newest_record->get_record_no());
               records.push_back(newest_record);
               for(requests_type::iterator rqi = requests.begin(); rqi != requests.end(); ++rqi)
               {
                  request_handle &request(*rqi);
                  request->set_expect_more_data(false);
               }
               std::copy(records.begin(), records.end(), std::back_inserter(cache));
               SinkBase::report_sink_records(source->get_manager(), requests, records);
               close_transaction();
            }
            else
            {
               state = state_collect_holes;
               reassign_transaction();
               transaction->request_focus();
               transaction->release_focus();
               std::copy(records.begin(), records.end(), std::back_inserter(cache));
               SinkBase::report_sink_records(source->get_manager(), requests, records);
            }
         } // on_date_range


         void OpDataRequest::on_collect_holes(cache_type &records)
         {
            // we will eliminate any records from the set that have already been collected.
            cache_type::iterator ri(
               std::remove_if(
                  records.begin(), records.end(), record_collected(collected_records)));
            if(ri != records.end())
            {
               std::copy(ri, records.end(), std::back_inserter(cache));
               records.erase(ri, records.end());
            }
            if(!requests.empty() && requests.front()->get_start_option() == Request::start_date_query)
            {
               request_handle &first(requests.front());
               ri = std::remove_if(
                  records.begin(), records.end(), record_outside_time(first->get_start_time(), first->get_end_time()));
               if(ri != records.end())
                  records.erase(ri, records.end());
            }

            // we can now add all of the remaining records to the collected records list
            for(cache_type::iterator ri = records.begin(); ri != records.end(); ++ri)
            {
               record_handle &record(*ri);
               collected_records.add_range(record->get_record_no(), record->get_record_no());
               expected_records.remove_range(record->get_record_no(), record->get_record_no());
            }

            // we can now decide whether to go on
            if(expected_records.empty() || records.empty())
            {
               // we may not have collected the entire table but, since the request is satisfied, we
               // will pretend in the collected_records structure that we have.  This will simplify
               // future hole collection attempts.
               close_transaction();
               records.push_back(newest_record);
               collected_records.clear();
               if(newest_record->get_record_no() >= table_symbol->get_table_size())
                  collected_records.add_range(newest_record->get_record_no() - table_symbol->get_table_size(), newest_record->get_record_no());
               else
                  collected_records.add_range(0, newest_record->get_record_no());
               collected_records.add_range(newest_record->get_record_no(), newest_record->get_record_no());
               state = state_not_started;
               for(requests_type::iterator rqi = requests.begin(); rqi != requests.end(); ++rqi)
               {
                  request_handle &request(*rqi);
                  request->set_expect_more_data(false);
               }
               std::copy(records.begin(), records.end(), std::back_inserter(cache));
               SinkBase::report_sink_records(source->get_manager(), requests, records);
               if(!requests.empty() && requests.front()->get_start_option() == Request::start_date_query)
                  source->remove_operation(this);
               else
                  close_transaction();
            }
            else
            {
               reassign_transaction();
               transaction->request_focus();
               transaction->release_focus();
               std::copy(records.begin(), records.end(), std::back_inserter(cache));
               SinkBase::report_sink_records(source->get_manager(), requests, records);
            }
         } // on_collect_holes


         /**
          * Defines an operation that implements the set value request.
          */
         class OpSetValue: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source for this operation.
             *
             * @param sink_ Specifies the sink that will be notified on completion.
             *
             * @param uri_ Specifies the URI of the value to be set.
             *
             * @param value_ Specifies the value to be set.
             */
            OpSetValue(Bmp5Source *source, SinkBase *sink_, StrUni const &uri_, ValueSetter const &value_);

            /**
             * Destructor
             */
            virtual ~OpSetValue();

            /**
             * Overloads the base class method to handle a failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Handles an incoming BMP5 message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloaded to handle the case where this operation has focus.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to describe this operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);
            
         private:
            /**
             * Specifies the sink that will receive notification on completion.
             */
            SinkBase *sink;

            /**
             * Specifies the URI that will identify the value to be changed.
             */
            StrUni const uri;

            /**
             * Specifies the value to be set.
             */
            ValueSetter const value;
         };


         OpSetValue::OpSetValue(
            Bmp5Source *source, SinkBase *sink_, StrUni const &uri_, ValueSetter const &value_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            value(value_)
         { }


         OpSetValue::~OpSetValue()
         { }


         void OpSetValue::on_failure(SourceTran *sender, SourceTran::failure_type failure)
         {
            if(SinkBase::is_valid_instance(sink))
               sink->on_set_complete(source->get_manager(), uri, SinkBase::set_outcome_communication_failed);
            source->remove_operation(this);
         } // on_failure


         void OpSetValue::on_bmp5_message(SourceTran *sender, bmp5_message_handle &message)
         {
            byte rcd(message->readByte());
            uint2 reboot_interval(0);
            SinkBase::set_outcome_type outcome(SinkBase::set_outcome_unknown);
            switch(rcd)
            {
            case 0:
               outcome = SinkBase::set_outcome_succeeded;
               break;

            case 1:
               outcome = SinkBase::set_outcome_logger_security_blocked;
               break;

            case 16:
               outcome = SinkBase::set_outcome_invalid_column_name;
               break;

            case 17:
               outcome = SinkBase::set_outcome_invalid_data_type;
               break;

            case 18:
               outcome = SinkBase::set_outcome_invalid_subscript;
               break;

            case 21:
               outcome = SinkBase::set_outcome_succeeded;
               reboot_interval = message->readUInt2();
               break;
            }
            if(SinkBase::is_valid_instance(sink))
               sink->on_set_complete(source->get_manager(), uri, outcome);
            source->remove_operation(this, reboot_interval * 1000);
         } // on_bmp5_message


         void OpSetValue::on_focus_start(SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            TableFieldUri wart(uri);
            
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::set_value_cmd);
            cmd->addUInt2(source->get_security_code());
            cmd->addAsciiZ(wart.get_table_name().to_utf8().c_str());
            switch(value.value_type)
            {
            case ValueSetter::value_type_bool:
               cmd->addByte(LgrBool);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addByte(value.value_variant.v_bool ? 0xff : 0);
               break;

            case ValueSetter::value_type_float:
               cmd->addByte(LgrIeee4);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addIeee4(value.value_variant.v_float);
               break;

            case ValueSetter::value_type_uint4:
               cmd->addByte(LgrUInt4);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addUInt4(value.value_variant.v_uint4);
               break;

            case ValueSetter::value_type_int4:
               cmd->addByte(LgrInt4);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addInt4(value.value_variant.v_int4);
               break;

            case ValueSetter::value_type_uint2:
               cmd->addByte(LgrUInt2);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addUInt2(value.value_variant.v_uint2);
               break;

            case ValueSetter::value_type_int2:
               cmd->addByte(LgrInt2);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addInt2(value.value_variant.v_int2);
               break;

            case ValueSetter::value_type_uint1:
               cmd->addByte(LgrUInt1);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addByte(value.value_variant.v_uint1);
               break;

            case ValueSetter::value_type_int1:
               cmd->addByte(LgrInt1);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2(1);
               cmd->addByte(value.value_variant.v_int1);
               break;

            case ValueSetter::value_type_string:
               cmd->addByte(LgrAsciiZ);
               cmd->addAsciiZ(wart.get_column_name().to_utf8().c_str());
               cmd->addUInt2((uint2)value.value_string.length());
               cmd->addAsciiZ(value.value_string.c_str());
               break;
            }
            sender->set_time_out(5000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start


         void OpSetValue::get_transaction_description(SourceTran *sender, std::ostream &desc)
         {
            desc << "set value\",\"" << uri;
         } // get_transaction_description


         /**
          * Defines an operation object implements the send file operation.
          */
         class OpSendFile: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source  that owns this operation.
             *
             * @param sink_ Specifies the application object that will receive completion notice.
             *
             * @param uri_ Specifies the URI that identifies the station.
             *
             * @param dest_file_name_ Specifies the name of the file on the datalogger.
             *
             * @param file_name_ Specifies the name of the file to transmit.
             */
            OpSendFile(
               Bmp5Source *source,
               SinkBase *sink_,
               StrUni const &uri_,
               StrUni const &dest_file_name_,
               StrUni const &file_name_);

            /**
             * Destructor
             */
            virtual ~OpSendFile();

            /**
             * Overloads the base class version to handle a failure notification.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to handle notification of transaction focus.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to describe the operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);

         private:
            /**
             * Transmits the next fragment to the datalogger.
             */
            void send_next();
            
         private:
            /**
             * Specifies the application object that will receive notice of completion.
             */
            SinkBase *sink;

            /**
             * Specifies the uri used to identify the station.
             */
            StrUni const uri;

            /**
             * Specifies the name of the file on the datalogger.
             */
            StrUni const dest_file_name;

            /**
             * Specifies the name of the local file to send.
             */
            StrUni const file_name;

            /**
             * Specifies the handle to the file to send.
             */
            FILE *input;

            /**
             * Specifies the buffer used to read file content.
             */
            byte rx_buff[1024];

            /**
             * Set to true if the last fragment has been sent.
             */
            bool last_sent;

            /**
             * Specifies the last command message that was sent.
             */
            bmp5_message_handle last_cmd;

            /**
             * Specifies the number of times that we have retried.
             */
            uint4 retry_count;
         };


         OpSendFile::OpSendFile(
            Bmp5Source *source,
            SinkBase *sink_,
            StrUni const &uri_,
            StrUni const &dest_file_name_,
            StrUni const &file_name_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            dest_file_name(dest_file_name_),
            file_name(file_name_),
            input(0),
            last_sent(false),
            retry_count(0)
         { }


         OpSendFile::~OpSendFile()
         {
            if(input)
               fclose(input);
            input = 0;
         }


         void OpSendFile::on_failure(SourceTran *sender, SourceTran::failure_type failure)
         {
            if(failure == Csi::PakBus::PakCtrl::DeliveryFailure::timed_out_or_resource_error &&
               last_cmd != 0 &&
               ++retry_count < 3)
            {
               send_next();
            }
            else
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_communication_failed);
               source->remove_operation(this);
            }
         } // on_failure


         void OpSendFile::on_bmp5_message(SourceTran *sender, bmp5_message_handle &message)
         {
            byte rcd(message->readByte());
            uint4 offset;
            uint4 space_left(0xffffffff);
            sender->clear_time_out();
            switch(rcd)
            {
            case 0:
               offset = message->readUInt4();
               if(message->whatsLeft() >= sizeof(space_left))
                  space_left = message->readUInt4();
               if(last_sent)
               {
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_success);
                  source->remove_operation(this);
               }
               else
               {
                  if(space_left >= Csi::file_length(input))
                  {
                     last_cmd.clear();
                     retry_count = 0;
                     send_next();
                  }
                  else
                  {
                     if(SinkBase::is_valid_instance(sink))
                        sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_logger_resource_error);
                     source->remove_operation(this);
                  }
               }
               break;

            case 1:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_logger_security_blocked);
               source->remove_operation(this);
               break;
               
            case 2:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_logger_resource_error);
               source->remove_operation(this);
               break;

            case 9:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_communication_failed);
               source->remove_operation(this);
               break;

            case 13:
            case 14:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_invalid_file_name);
               source->remove_operation(this);
               break;
               
            case 20:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_logger_root_full);
               source->remove_operation(this);
               break;

            default:
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_unknown);
               source->remove_operation(this);
               break;
            }
         } // on_bmp5_message


         void OpSendFile::on_focus_start(SourceTran *sender)
         {
            // if the file is not open, we need to open it.
            if(input == 0)
               input = Csi::open_file(file_name, L"rb");
            if(input != 0)
               send_next();
            else
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_send_file_complete(source->get_manager(), uri, file_name, SinkBase::send_file_invalid_file_name);
               source->remove_operation(this);
            }
         } // on_focus_start


         void OpSendFile::get_transaction_description(SourceTran *sender, std::ostream &desc)
         {
            desc << "send file\",\"" << dest_file_name;
         } // get_transaction_description


         void OpSendFile::send_next()
         {
            if(last_cmd != 0)
            {
               transaction->set_time_out(10000);
               transaction->send_bmp5_message(last_cmd);
            }
            else
            {
               bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
               uint4 max_body_len(
                  source->get_router()->get_max_body_len(transaction->get_destination_address()) - 9);
               StrAsc dest_file(dest_file_name.to_utf8());
               int8 current_pos(Csi::file_tell(input));
               uint4 fragment_len;
               
               cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_send_cmd);
               cmd->addUInt2(source->get_security_code());
               if(current_pos == 0)
               {
                  max_body_len -= (uint4)dest_file.length();
                  cmd->addAsciiZ(dest_file.c_str());
               }
               else
                  cmd->addAsciiZ("");
               cmd->addByte(0);    // reserved
               fragment_len = (uint4)fread(rx_buff, 1, max_body_len, input);
               cmd->addByte(fragment_len == max_body_len ? 0 : 1);
               cmd->addUInt4((uint4)current_pos);
               cmd->addBytes(rx_buff, fragment_len);
               last_sent = fragment_len < max_body_len;
               if(last_sent)
               {
                  fclose(input);
                  input = 0;
               }
               last_cmd = cmd;
               transaction->set_time_out(10000);
               transaction->send_bmp5_message(cmd);
            }
         } // send_next


         /**
          * Defines an operation that is used to first query the logger directory for the newest
          * file that matches a given pattern and, if found, will read the contents of that file to
          * be sent to the sink.
          */
         class OpNewestFile: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the application object that will receive notification of
             * completion.
             *
             * @param uri_ Specifies the station URI.
             *
             * @param pattern_ Specifies the pattern for which we will search.
             */
            OpNewestFile(
               Bmp5Source *source, SinkBase *sink_, StrUni const &uri_, StrUni const &pattern_);

            /**
             * Destructor
             */
            virtual ~OpNewestFile();

            /**
             * Overloads the base class version to handle a transaction failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to handle the case where the initial focus has been
             * received.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to describe the operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);

         private:
            /**
             * Called to handle a received fragment of the directory.
             */
            void on_get_dir(bmp5_message_handle &message);

            /**
             * Called to handle a received fragment of the selected file.
             */
            void on_read_file(bmp5_message_handle &message);

         private:
            /**
             * Specifies the application object that will receive notification of completion and
             * progress.
             */
            SinkBase *sink;

            /**
             * Specifies the station URI.
             */
            StrUni const &uri;

            /**
             * Specifies the pattern used to search for the file.
             */
            StrUni const pattern;
            
            /**
             * Represents the state of this operation.
             */
            enum state_type
            {
               state_get_dir,
               state_read_file
            } state;

            /**
             * Specifies a buffer used to parse the directory contents.
             */
            Csi::PakBus::Bmp5Message dir_buff;

            /**
             * Specifies a buffer used to read file contents.
             */
            StrBin rx_buff;

            /**
             * Specifies the pattern string sent to the datalogger.
             */
            StrAsc logger_pattern;

            /**
             * Specifies the logger file that has been selected to be returned.
             */
            StrAsc file_name;

            /**
             * Specifies the last swath that was requested.  This will be used to evaluate whether
             * the file receive operation is complete.
             */
            uint2 last_swath;

            /**
             * Specifies the number of bytes that have been received.
             */
            uint4 bytes_received;
         };


         OpNewestFile::OpNewestFile(
            Bmp5Source *source, SinkBase *sink_, StrUni const &uri_, StrUni const &pattern_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            pattern(pattern_),
            state(state_get_dir),
            bytes_received(0)
         {
            if(pattern.find(L"*") < pattern.length() || pattern.find(L"?") < pattern.length())
            {
               logger_pattern = ".DIR2";
               if(pattern.length() > 0)
               {
                  StrAsc pattern_mb(pattern.to_utf8());
                  pattern_mb.replace("/", ":");
                  logger_pattern.append("/");
                  logger_pattern.append(pattern_mb);
               }
            }
            else
            {
               state = state_read_file;
               file_name = pattern.to_utf8();
               file_name.replace("/", ":");
            }
         } // constructor


         OpNewestFile::~OpNewestFile()
         { }


         void OpNewestFile::on_failure(
            SourceTran *sender, SourceTran::failure_type failure)
         {
            if(SinkBase::is_valid_instance(sink))
               sink->on_get_newest_file_status(source->get_manager(), SinkBase::get_newest_status_communication_failed, uri, pattern);
            source->remove_operation(this);
         } // on_failure


         void OpNewestFile::on_focus_start(SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
            cmd->addUInt2(source->get_security_code());
            last_swath = (uint2)source->get_router()->get_max_body_len(sender->get_destination_address()) - 5;
            if(state == state_get_dir)
               cmd->addAsciiZ(logger_pattern.c_str());
            else
               cmd->addAsciiZ(file_name.c_str());
            cmd->addByte(0);
            cmd->addUInt4(0);
            cmd->addUInt2(last_swath);
            sender->set_time_out(10000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start


         void OpNewestFile::on_bmp5_message(
            SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::please_wait_notification)
            {
               byte message_type(message->readByte());
               uint4 estimate_sec(message->readUInt2());
               sender->set_time_out(10000 + estimate_sec * 1000);
            }
            else if(state == state_get_dir)
               on_get_dir(message);
            else if(state == state_read_file)
               on_read_file(message);
         } // on_bmp5_message


         void OpNewestFile::get_transaction_description(
            SourceTran *sender, std::ostream &desc)
         {
            desc << "newest file\",\"" << pattern;
         } // get_transaction_description


         void OpNewestFile::on_get_dir(bmp5_message_handle &message)
         {
            byte rcd(message->readByte());
            uint4 offset(message->readUInt4());
            if(rcd == 0)
            {
               message->readBytes(rx_buff, message->whatsLeft());
               dir_buff.addBytes(rx_buff.getContents(), (uint4)rx_buff.length());
               if(rx_buff.length() == last_swath)
               {
                  bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                  cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
                  cmd->addUInt2(source->get_security_code());
                  cmd->addAsciiZ(logger_pattern.c_str());
                  cmd->addByte(0); // close flag
                  cmd->addUInt4(dir_buff.whatsLeft());
                  cmd->addUInt2(last_swath);
                  transaction->set_time_out(10000);
                  transaction->send_bmp5_message(cmd);
               }
               else
               {
                  // we can now attempt to parse the directory
                  byte dir_version(dir_buff.readByte());
                  StrAsc current_file_name;
                  Csi::LgrDate current_file_time;
                  int8 file_size;
                  StrAsc file_name;
                  StrAsc file_time_s;
                  Csi::LgrDate file_time;
                  byte attrib;
                  StrAsc expr(pattern.to_utf8());

                  expr.replace("/", ":");
                  while(dir_buff.whatsLeft())
                  {
                     dir_buff.readAsciiZ(file_name);
                     if(dir_version >= 2 && file_name.last() == ':')
                        file_size = (int8)dir_buff.readIeee4();
                     else
                        file_size = dir_buff.readUInt4();
                     dir_buff.readAsciiZ(file_time_s);
                     file_time = Csi::LgrDate::fromStr(file_time_s.c_str());
                     while((attrib = dir_buff.readByte()) != 0)
                        0;
                     if(file_name.last() != ':' &&
                        Csi::matches_wildcard_expr(file_name, expr) &&
                        file_time > current_file_time)
                     {
                        current_file_time = file_time;
                        current_file_name = file_name;
                     }
                  }

                  // we will have either found the newest matching file or no file at all.
                  if(current_file_name.length() > 0)
                  {
                     bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                     cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
                     cmd->addUInt2(source->get_security_code());
                     cmd->addAsciiZ(current_file_name.c_str());
                     cmd->addByte(0); // close flag
                     cmd->addUInt4(0); // offset
                     cmd->addUInt2(last_swath);
                     bytes_received = 0;
                     file_name = current_file_name;
                     transaction->set_time_out(10000);
                     state = state_read_file;
                     transaction->send_bmp5_message(cmd);
                  }
                  else
                  {
                     if(SinkBase::is_valid_instance(sink))
                        sink->on_get_newest_file_status(
                           source->get_manager(), SinkBase::get_newest_status_no_file, uri, pattern);
                     source->remove_operation(this);
                  }
               }
            }
            else
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_get_newest_file_status(
                     source->get_manager(), SinkBase::get_newest_status_logger_permission_denied, uri, pattern);
               source->remove_operation(this);
            }
         } // on_get_dir


         void OpNewestFile::on_read_file(bmp5_message_handle &message)
         {
            byte rcd(message->readByte());
            if(rcd == 0)
            {
               uint4 offset(message->readUInt4());
               if(offset == bytes_received)
               {
                  SinkBase::get_newest_file_status_type outcome;
                  message->readBytes(rx_buff, message->whatsLeft());
                  if(rx_buff.length() == last_swath)
                     outcome = SinkBase::get_newest_status_in_progress;
                  else
                     outcome = SinkBase::get_newest_status_complete;
                  if(SinkBase::is_valid_instance(sink))
                  {
                     sink->on_get_newest_file_status(
                        source->get_manager(), outcome, uri, pattern, file_name, rx_buff.getContents(), (uint4)rx_buff.length());
                  }
                  if(outcome == SinkBase::get_newest_status_in_progress)
                  {
                     bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                     bytes_received += (uint4)rx_buff.length();
                     cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
                     cmd->addUInt2(source->get_security_code());
                     cmd->addAsciiZ(file_name.c_str());
                     cmd->addByte(0);
                     cmd->addUInt4(bytes_received);
                     cmd->addUInt2(last_swath);
                     transaction->set_time_out(10000);
                     transaction->send_bmp5_message(cmd);
                  }
                  else
                     source->remove_operation(this);
               }
            }
            else
            {
               SinkBase::get_newest_file_status_type outcome(SinkBase::get_newest_status_unknown_failure);
               switch(rcd)
               {
               case 1:
                  outcome = SinkBase::get_newest_status_logger_permission_denied;
                  break;

               case 13:
               case 14:
                  outcome = SinkBase::get_newest_status_invalid_file_name;
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
                  sink->on_get_newest_file_status(source->get_manager(), outcome, uri, pattern);
               source->remove_operation(this);
            }
         } // on_read_file


         /**
          * Defines an operation that implements the clock.
          */
         class OpClock: public SourceOperation
         {
         public:
            /**
             * Constructor.
             *
             * @param source Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the application object that will receive notification of
             * completion.
             *
             * @param uri_ Specifies the station URI.
             *
             * @param should_set_ Set to true if the clock should be set.
             *
             * @param send_server_time_ Set to true if the server time is client specified.
             *
             * @param server_time_ Specifies the server time to set.
             */
            OpClock(
               Bmp5Source *source,
               SinkBase *sink_,
               StrUni const &uri_,
               bool should_set_,
               bool send_server_time_,
               Csi::LgrDate const &server_time_);

            /**
             * Destructor
             */
            virtual ~OpClock();

            /**
             * Overloads the base class version to handle the focus set event.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to handle the message received event.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to handle a transaction failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to format the description.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);
            
         private:
            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the station URI.
             */
            StrUni const uri;

            /**
             * Set to true if the clock should be set.
             */
            bool const should_set;

            /**
             * Set to true if the application specified time should be used.
             */
            bool const send_server_time;

            /**
             * Specifies the server time that should be used.
             */
            Csi::LgrDate const server_time;

            /**
             * Specifies the state of this operation.
             */
            enum state_type
            {
               state_check,
               state_set
            } state;
         };


         OpClock::OpClock(
            Bmp5Source *source,
            SinkBase *sink_,
            StrUni const &uri_,
            bool should_set_,
            bool send_server_time_,
            Csi::LgrDate const &server_time_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            should_set(should_set_),
            send_server_time(send_server_time_),
            server_time(server_time_),
            state(state_check)
         { }


         OpClock::~OpClock()
         { }


         void OpClock::on_focus_start(SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::clock_check_set_ex_cmd);
            cmd->addUInt2(source->get_security_code());
            cmd->addUInt4(0);
            cmd->addUInt4(0);
            sender->set_time_out(5000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start


         void OpClock::on_bmp5_message(SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::clock_check_set_ex_ack)
            {
               byte rcd(message->readByte());
               Csi::LgrDate logger_clock;
               Csi::LgrDate now(Csi::LgrDate::system() - sender->get_round_trip_time() * Csi::LgrDate::nsecPerMSec);
               if(send_server_time)
                  now = server_time;
               if(rcd == 0)
               {
                  logger_clock = message->readNSec() - sender->get_round_trip_time() * Csi::LgrDate::nsecPerMSec;
                  if(state == state_check && should_set)
                  {
                     bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                     cmd->set_message_type(Csi::PakBus::Bmp5Messages::clock_check_set_ex_cmd);
                     cmd->addUInt2(source->get_security_code());
                     cmd->addNSec(now - logger_clock);
                     sender->set_time_out(5000);
                     state = state_set;
                     sender->send_bmp5_message(cmd);
                  }
                  else
                  {
                     if(SinkBase::is_valid_instance(sink))
                     {
                        sink->on_clock_complete(
                           source->get_manager(),
                           uri,
                           state == state_check ? SinkBase::clock_success_checked : SinkBase::clock_success_set,
                           logger_clock);
                     }
                     source->remove_operation(this);
                  }
               }
               else
               {
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_clock_complete(source->get_manager(), uri, SinkBase::clock_failure_logger_permission);
                  source->remove_operation(this);
               }
            }
         } // on_bmp5_message


         void OpClock::on_failure(SourceTran *sender, SourceTran::failure_type failure)
         {
            if(SinkBase::is_valid_instance(sink))
               sink->on_clock_complete(source->get_manager(), uri, SinkBase::clock_failure_communication);
            source->remove_operation(this);
         } // on_failure


         void OpClock::get_transaction_description(
            SourceTran *sender, std::ostream &desc)
         {
            if(state == state_check)
               desc << "clock check";
            else
               desc << "clock set";
         }


         /**
          * Defines an object that implements the file control protocol for a BMP5 source.
          */
         class OpFileControl: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the application object that will receive notice of completion.
             *
             * @param uri_ Specifies the station URI.
             *
             * @param command_ Specifies the file control command code.
             *
             * @param p1_ Specifies the first argument.
             *
             * @param p2_ Specifies the second argument.
             */
            OpFileControl(
               Bmp5Source *source,
               SinkBase *sink_,
               StrUni const &uri_,
               uint4 command_,
               StrAsc const &p1_,
               StrAsc const &p2_);

            /**
             * Destructor
             */
            virtual ~OpFileControl();

            /**
             * Overloads the base class version to handle acceptance of focus.
             */
            virtual void on_focus_start(SourceTran *sender);

            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to handle a failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to write a description of this operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);

         private:
            /**
             * Specifies the application object that will receive notification of completion.
             */
            SinkBase *sink;

            /**
             * Specifies the station URI.
             */
            StrUni const uri;

            /**
             * Specifies the command control code.
             */
            uint4 const command;

            /**
             * Specifies the first file parameter.
             */
            StrAsc const p1;

            /**
             * Specifies the second file parameter.
             */
            StrAsc const p2;
         };


         OpFileControl::OpFileControl(
            Bmp5Source *source,
            SinkBase *sink_,
            StrUni const &uri_,
            uint4 command_,
            StrAsc const &p1_,
            StrAsc const &p2_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            command(command_),
            p1(p1_),
            p2(p2_)
         { }


         OpFileControl::~OpFileControl()
         { }


         void OpFileControl::on_focus_start(
            SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_control_cmd);
            cmd->addUInt2(source->get_security_code());
            cmd->addAsciiZ(p1.c_str());
            cmd->addByte((byte)command);
            cmd->addAsciiZ(p2.c_str());
            sender->set_time_out(10000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start


         void OpFileControl::on_bmp5_message(
            SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::please_wait_notification)
            {
               byte message_type(message->readByte());
               uint4 estimate_sec(message->readUInt2());
               sender->set_time_out(estimate_sec * Csi::LgrDate::msecPerSec + 10000);
            }
            else if(message->get_message_type() == Csi::PakBus::Bmp5Messages::file_control_ack)
            {
               byte rcd(message->readByte());
               uint2 hold_off(0);
               SinkBase::file_control_outcome_type outcome(SinkBase::filecontrol_failure_unknown);
               switch(rcd)
               {
               case 0:
                  outcome = SinkBase::filecontrol_success;
                  if(message->whatsLeft() >= 2)
                     hold_off = message->readUInt2();
                  break;

               case 1:
                  outcome = SinkBase::filecontrol_failure_logger_permission;
                  break;

               case 13:
                  outcome = SinkBase::filecontrol_failure_invalid_file_name;
                  break;

               case 19:
                  outcome = SinkBase::filecontrol_failure_unsupported_command;
                  break;

               case 20:
                  outcome = SinkBase::filecontrol_failure_logger_root_dir_full;
                  break;

               case 22:
                  outcome = SinkBase::filecontrol_failure_file_busy;
                  break;

               case 23:
                  outcome = SinkBase::filecontrol_success;
                  hold_off = message->readUInt2();
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
                  sink->on_file_control_complete(
                     source->get_manager(), uri, command, p1, p2, outcome, hold_off);
               source->remove_operation(this, hold_off * Csi::LgrDate::msecPerSec);
            }
         } // on_bmp5_message


         void OpFileControl::on_failure(
            SourceTran *sender, SourceTran::failure_type failure)
         {
            if(SinkBase::is_valid_instance(sink))
            {
               sink->on_file_control_complete(
                  source->get_manager(), uri, command, p1, p2, SinkBase::filecontrol_failure_communication);
            }
            source->remove_operation(this);
         } // on_failure


         void OpFileControl::get_transaction_description(
            SourceTran *sender, std::ostream &desc)
         {
            desc << "file control\",\"" << command << "\",\"" << p1 << "\",\"" << p2;
         }


         /**
          * Defines an operation that carries out terminal services.
          */
         class OpTerminal: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the terminal sink.
             *
             * @param uri_ Specifies the station uri.
             *
             * @param sink_token_ Specifies the token for this sink.
             */
            OpTerminal(
               Bmp5Source *source,
               TerminalSinkBase *sink_,
               StrUni const &uri_,
               int8 sink_token_);

            /**
             * Destructor
             */
            virtual ~OpTerminal();

            /**
             * Overloads the base class version as a no-op because this operation will not use
             * focus.
             */
            virtual void on_focus_start(SourceTran *sender)
            { }

            /**
             * Overloads the start method to generate the PakBus transaction.
             */
            virtual void start();

            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);

            /**
             * Overloads the base class version to handle a transaction failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);

            /**
             * Overloads the base class version to write a description of this operation.
             */
            virtual void get_transaction_description(
               SourceTran *sender, std::ostream &desc)
            { desc << "terminal"; }

            /**
             * @return Overloads the base class version to indicate that this is a terminal.
             */
            virtual bool supports_terminal() const
            { return true; }

            /**
             * Adds the specified data to the buffer and prepares to send the next BMP5 message.
             */
            void send_data(void const *buff, size_t buff_len);

         private:
            /**
             * Sends the next fragment to the datalogger.
             */
            void send_next();

         private:
            /**
             * Specifies the data to be written to the datalogger.
             */
            Csi::ByteQueue tx_buff;

            /**
             * Specifies the terminal sink object.
             */
            TerminalSinkBase *sink;

            /**
             * Specifies the sink's token.
             */
            int8 const sink_token;

            /**
             * Specifies the station uri.
             */
            StrUni const uri;

            /**
             * Buffers the received data.
             */
            StrBin rx_buff;

            friend class Cora::DataSources::Bmp5Source;
         };


         OpTerminal::OpTerminal(
            Bmp5Source *source,
            TerminalSinkBase *sink_,
            StrUni const &uri_,
            int8 sink_token_):
            SourceOperation(source),
            sink(sink_),
            uri(uri_),
            sink_token(sink_token_)
         { }


         OpTerminal::~OpTerminal()
         { }


         void OpTerminal::start()
         {
            transaction.bind(new SourceTran(this));
            transaction->set_time_out(40000);
            source->get_router()->open_transaction(transaction.get_handle());
         } // start


         void OpTerminal::on_bmp5_message(
            SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::user_io_notification)
            {
               byte rcd(message->readByte());
               if(rcd == 0)
               {
                  rx_buff.cut(0);
                  if(message->whatsLeft() > 0)
                  {
                     message->readBytes(rx_buff, message->whatsLeft());
                     if(TerminalSinkBase::is_valid_instance(sink))
                     {
                        sink->on_terminal_content(source->get_manager(), sink_token, rx_buff);
                        sender->reset_time_out();
                        send_next();
                     }
                     else
                        source->remove_operation(this);
                  }
               }
               else
               {
                  if(TerminalSinkBase::is_valid_instance(sink))
                     sink->on_terminal_failed(
                        source->get_manager(),
                        sink_token,
                        TerminalSinkBase::terminal_failure_logger_security_blocked);
                  source->remove_operation(this);
               }
            }
         } // on_bmp5_message


         void OpTerminal::on_failure(
            SourceTran *sender, SourceTran::failure_type failure)
         {
            if(TerminalSinkBase::is_valid_instance(sink))
               sink->on_terminal_failed(source->get_manager(), sink_token, TerminalSinkBase::terminal_failure_communication);
            source->remove_operation(this);
         } // on_failure


         void OpTerminal::send_data(void const *buff, size_t buff_len)
         {
            tx_buff.push(buff, (uint4)buff_len);
            send_next();
         } // send_data


         void OpTerminal::send_next()
         {
            if(tx_buff.size() > 0)
            {
               bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
               uint4 swath_len(
                  Csi::csimin(
                     tx_buff.size(),
                     source->get_router()->get_max_body_len(source->get_pakbus_address()) - 2));
               cmd->set_message_type(Csi::PakBus::Bmp5Messages::user_io_cmd);
               cmd->addUInt2(source->get_security_code());
               rx_buff.cut(0);
               tx_buff.pop(rx_buff, swath_len);
               cmd->addBytes(rx_buff.getContents(), swath_len);
               transaction->send_bmp5_message(cmd);
               transaction->reset_time_out();
            }
         } // send_next


         /**
          * Defines an operation that attempts to determine our access level with the datalogger.
          */
         class OpCheckAccessLevel: public SourceOperation
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the sink.
             */
            OpCheckAccessLevel(Bmp5Source *source_, SinkBase *sink_);
            
            /**
             * Destructor
             */
            virtual ~OpCheckAccessLevel();
            
            /**
             * Overloads the base class version to handle the focus set event.
             */
            virtual void on_focus_start(SourceTran *sender);
            
            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);
            
            /**
             * Overloads the base class version to handle a transaction failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure);
            
            /**
             * Overloads the base class version to format the description for this operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &desc);
            
         private:
            /**
             * Specifies the sink.
             */
            SinkBase *sink;
            
            /**
             * Specifies the access level that we have determined.
             */
            Csi::PasswdHelpers::access_level_type access_level;
         };
         
         
         OpCheckAccessLevel::OpCheckAccessLevel(Bmp5Source *source, SinkBase *sink_):
            SourceOperation(source),
            sink(sink_),
            access_level(Csi::PasswdHelpers::access_none)
         { }
         
         
         OpCheckAccessLevel::~OpCheckAccessLevel()
         { }
         
         
         void OpCheckAccessLevel::on_focus_start(SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::check_access_cmd);
            cmd->addUInt2(source->get_security_code());
            sender->set_time_out(5000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start
         
         
         void OpCheckAccessLevel::on_bmp5_message(SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::check_access_ack)
            {
               byte rcd(message->readByte());
               switch(rcd)
               {
               default:
               case 0:
                  access_level = Csi::PasswdHelpers::access_none;
                  break;

               case 1:
                  access_level = Csi::PasswdHelpers::access_all;
                  break;

               case 2:
                  access_level = Csi::PasswdHelpers::access_read_write;
                  break;

               case 3:
                  access_level = Csi::PasswdHelpers::access_read_only;
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
                  sink->on_check_access_level_complete(
                     source->get_manager(),
                     SinkBase::check_access_level_outcome_success,
                     access_level);
               source->remove_operation(this);
            }
            else if(message->get_message_type() == Csi::PakBus::Bmp5Messages::clock_check_set_ex_ack)
            {
               byte rcd(message->readByte());
               if(rcd == 0)
               {
                  bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                  cmd->set_message_type(Csi::PakBus::Bmp5Messages::set_value_cmd);
                  cmd->addUInt2(source->get_security_code());
                  cmd->addAsciiZ("Status");
                  cmd->addByte(LgrInt4);
                  cmd->addAsciiZ("OSVersion");
                  cmd->addUInt2(1);
                  cmd->addInt4(1);
                  sender->send_bmp5_message(cmd);
                  access_level = Csi::PasswdHelpers::access_read_only;
               }
               else
               {
                  SinkBase::check_access_level_outcome_type outcome(SinkBase::check_access_level_outcome_success);
                  if(rcd != 1)
                     outcome = SinkBase::check_access_level_outcome_failure_communication;
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_check_access_level_complete(source->get_manager(), outcome, access_level);
                  source->remove_operation(this);
               }
            }
            else if(message->get_message_type() == Csi::PakBus::Bmp5Messages::set_value_ack)
            {
               byte rcd(message->readByte());
               SinkBase::check_access_level_outcome_type outcome(SinkBase::check_access_level_outcome_success);
               if(rcd != 1)
               {
                  bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                  cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_control_cmd);
                  cmd->addUInt2(source->get_security_code());
                  cmd->addAsciiZ("");
                  cmd->addByte(90); // hopefully, this is a no-op.
                  cmd->addAsciiZ("");
                  sender->send_bmp5_message(cmd);
                  access_level = Csi::PasswdHelpers::access_read_write;
               }
               else
               {
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_check_access_level_complete(source->get_manager(), outcome, access_level);
                  source->remove_operation(this);
               }
            }
            else if(message->get_message_type() == Csi::PakBus::Bmp5Messages::file_control_ack)
            {
               byte rcd(message->readByte());
               if(rcd != 1)
                  access_level = Csi::PasswdHelpers::access_all;
               if(SinkBase::is_valid_instance(sink))
                  sink->on_check_access_level_complete(source->get_manager(), SinkBase::check_access_level_outcome_success, access_level);
               source->remove_operation(this);
            }
         } // on_bmp5_message
         
         
         void OpCheckAccessLevel::on_failure(SourceTran *sender, SourceTran::failure_type failure)
         {
            if(failure == Csi::PakBus::PakCtrl::DeliveryFailure::unsupported_message_type)
            {
               bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
               cmd->set_message_type(Csi::PakBus::Bmp5Messages::clock_check_set_ex_cmd);
               cmd->addUInt2(source->get_security_code());
               cmd->addUInt4(0);
               cmd->addUInt4(0);
               sender->set_time_out(5000);
               sender->send_bmp5_message(cmd);
            }
            else
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_check_access_level_complete(
                     source->get_manager(),
                     SinkBase::check_access_level_outcome_failure_communication,
                     access_level);
               source->remove_operation(this);
            }
         } // on_failure
         
         
         void OpCheckAccessLevel::get_transaction_description(SourceTran *sender, std::ostream &desc)
         { desc << "check access level"; }

         
         /**
          * Defines the operation type used to get a files list from the datalogger.
          */
         class OpListFiles: public SourceOperation
         {
         private:
            /**
             * Specifies the sink.
             */
            SinkBase *sink;
            
            /**
             * Specifies the station uri.
             */
            StrUni const station_uri;
            
            /**
             * Specifies the transaction.
             */
            int8 transaction;
            
            /**
             * Specifies the filter.
             */
            StrAsc filter;
            
            /**
             * Specifies the buffer that is used to buffer fragments of the directory listing received
             * from the datalogger.
             */
            Csi::PakBus::Bmp5Message dir_buff;
            
            /**
             * Specifies the last swath that was requested.  This will be used to determine whether the
             * transmission is complete.
             */
            uint2 last_swath;
            
            /**
             * Specifies the file name being requested.
             */
            StrAsc file_name;
            
            /**
             * Used to facilitate transfer between buffers.
             */
            StrBin rx_buff;
            
         public:
            /**
             * Constructor
             *
             * @param source Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the sink.
             *
             * @param station_uri_ Specifies the station URI.
             *
             * @param transaction_ Specifies the transaction.
             *
             * @param filter_ Specifies the filter.
             */
            OpListFiles(
               Bmp5Source *source,
               SinkBase *sink_,
               StrUni const &station_uri_,
               int8 transaction_,
               StrAsc const &filter_):
               SourceOperation(source),
               sink(sink_),
               station_uri(station_uri_),
               transaction(transaction_),
               filter(filter_),
               last_swath(0),
               file_name(".DIR2")
            {
               if(filter.length() > 0)
               {
                  file_name.append('/');
                  filter.replace("/", ":");
                  file_name.append(filter);
               }
            }
            
            /**
             * Destructor
             */
            virtual ~OpListFiles()
            { }
            
            /**
             * Overloads the base class version to start the operation.
             */
            virtual void on_focus_start(SourceTran *tran);
            
            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void on_bmp5_message(SourceTran *sender, bmp5_message_handle &message);
            
            /**
             * Overloads the bae class version to report a failure.
             */
            virtual void on_failure(SourceTran *sender, SourceTran::failure_type failure)
            {
               if(SinkBase::is_valid_instance(sink))
               {
                  sink->on_list_files_complete(
                     source->get_manager(),
                     SinkBase::list_files_outcome_failure_comms,
                     station_uri,
                     transaction,
                     filter,
                     SinkBase::files_type());
               }
               source->remove_operation(this);
            }
            
            /**
             * Overloads the base class version to format a description of this operation.
             */
            virtual void get_transaction_description(SourceTran *sender, std::ostream &out)
            { out << "list logger files"; }
         };
         
         
         void OpListFiles::on_focus_start(SourceTran *sender)
         {
            bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
            cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
            cmd->addUInt2(source->get_security_code());
            last_swath = (uint2)source->get_router()->get_max_body_len(sender->get_destination_address()) - 5;
            cmd->addAsciiZ(file_name.c_str());
            cmd->addByte(0);
            cmd->addUInt4(0);
            cmd->addUInt2(last_swath);
            sender->set_time_out(10000);
            sender->send_bmp5_message(cmd);
         } // on_focus_start
         
         
         void OpListFiles::on_bmp5_message(SourceTran *sender, bmp5_message_handle &message)
         {
            if(message->get_message_type() == Csi::PakBus::Bmp5Messages::please_wait_notification)
            {
               byte message_type(message->readByte());
               uint4 estimate_sec(message->readUInt2());
               sender->set_time_out(10000 + estimate_sec * 1000);
            }
            else
            {
               byte rcd(message->readByte());
               uint4 offset(message->readUInt4());
               if(rcd == 0)
               {
                  // we'll ignore any repeated fragments
                  if(offset == dir_buff.whatsLeft())
                  {
                     message->readBytes(rx_buff, message->whatsLeft());
                     dir_buff.addBytes(rx_buff.getContents(), (uint4)rx_buff.length());
                     if(rx_buff.length() == last_swath)
                     {
                        bmp5_message_handle cmd(new Csi::PakBus::Bmp5Message);
                        cmd->set_message_type(Csi::PakBus::Bmp5Messages::file_receive_cmd);
                        cmd->addUInt2(source->get_security_code());
                        cmd->addAsciiZ(file_name.c_str());
                        cmd->addByte(0); // close flag
                        cmd->addUInt4(dir_buff.whatsLeft());
                        cmd->addUInt2(last_swath);
                        sender->set_time_out(10000);
                        sender->send_bmp5_message(cmd);
                     }
                     else
                     {
                        byte dir_version(dir_buff.readByte());
                        SinkBase::files_type files;
                        while(dir_buff.whatsLeft() > 0)
                        {
                           SinkBase::list_file_type file;
                           StrAsc file_name;
                           int8 file_size;
                           byte attrib;
                           StrAsc last_update;

                           dir_buff.readAsciiZ(file_name);
                           file.set_name(file_name);
                           if(dir_version >= 2 && file_name.last() == ':')
                              file_size = (int8)dir_buff.readIeee4();
                           else
                              file_size = dir_buff.readUInt4();
                           file.set_attr_file_size(file_size);
                           dir_buff.readAsciiZ(last_update);
                           file.set_attr_last_update(last_update);
                           file.set_attr_run_now(false);
                           file.set_attr_run_on_power_up(false);
                           while((attrib = dir_buff.readByte()) != 0)
                           {
                              switch(attrib)
                              {
                              case 1:
                                 file.set_attr_run_now(true);
                                 break;
                                 
                              case 2:
                                 file.set_attr_run_on_power_up(true);
                                 break;
                                 
                              case 3:
                                 file.set_attr_read_only(true);
                                 break;
                                 
                              case 5:
                                 file.set_attr_paused(true);
                                 break;
                              }
                           }
                           files.push_back(file);
                        }
                        if(SinkBase::is_valid_instance(sink))
                        {
                           sink->on_list_files_complete(
                              source->get_manager(),
                              SinkBase::list_files_outcome_success,
                              station_uri,
                              transaction,
                              filter,
                              files);
                        }
                        source->remove_operation(this);
                     }
                  }
               }
               else
               {
                  if(SinkBase::is_valid_instance(sink))
                  {
                     SinkBase::list_files_outcome_type outcome(SinkBase::list_files_outcome_failure_unknown);
                     if(rcd == 1)
                        outcome = SinkBase::list_files_outcome_failure_logger_security;
                     sink->on_list_files_complete(
                        source->get_manager(),
                        outcome,
                        station_uri,
                        transaction,
                        filter,
                        SinkBase::files_type());
                  }
                  source->remove_operation(this);
               }
            }
         } // on_bmp5_message
      };
      
      namespace
      {
         StrUni const serial_port_name_name(L"serial-port");
         StrUni const baud_rate_name(L"baud-rate");
         StrUni const use_tcp_port_name(L"use-tcp-port");
         StrUni const tcp_server_address_name(L"tcp-server-address");
         StrUni const tcp_server_port_name(L"tcp-server-port");
         StrUni const pakbus_address_name(L"pakbus-address");
         StrUni const my_address_name(L"my-pakbus-address");
         StrUni const neighbour_address_name(L"neighbour-address");
         StrUni const pakbus_encryption_key_name(L"pakbus-encryption-key");
         StrUni const security_code_name(L"security-code");
         StrUni const poll_interval_name(L"poll-interval");
         StrUni const poll_base_name(L"poll-base");
         StrUni const log_name(L"log");
         StrUni const log_path_name(L"path");
         StrUni const log_file_name(L"file");
         StrUni const log_bale_size_name(L"bale-size");
         StrUni const log_bale_count_name(L"bale-count");
         StrUni const log_time_based_baling_name(L"time-based");
         StrUni const log_time_based_interval_name(L"time-based-interval");
      };

      
      Bmp5Source::Bmp5Source(StrUni const &name):
         SourceBase(name),
         baud_rate(115200),
         my_pakbus_address(4080),
         pakbus_address(0),
         neighbour_address(0),
         connect_state(connect_state_offline),
         security_code(0),
         retry_id(0),
         compile_state(compile_no_program),
         os_sig(0),
         program_sig(0),
         retry_requests_id(0),
         poll_schedule_id(0),
         disable_comms_id(0),
         tcp_server_port(6785),
         use_tcp_port(false)
      {
         source_symbol.bind(new Bmp5SourceHelpers::SourceSymbol(this));
      } // constructor


      Bmp5Source::~Bmp5Source()
      {
         router.clear();
         serial_port.clear();
         tcp_port.clear();
         if(timer != 0 && retry_id)
            timer->disarm(retry_id);
         if(timer != 0 && retry_requests_id)
            timer->disarm(retry_requests_id);
         if(timer != 0 && disable_comms_id)
            timer->disarm(disable_comms_id);
         timer.clear();
         log.clear();
         if(scheduler != 0 && poll_schedule_id)
         {
            scheduler->cancel(poll_schedule_id);
            poll_schedule_id = 0;
         }
         scheduler.clear();
      } // destructor


      void Bmp5Source::connect()
      {
         if(log_enabled && log_path.length() > 0 && log_file.length() > 0)
         {
            log.bind(new Csi::LogByte(log_path.c_str(), log_file.c_str()));
            log->setBaleParams(log_bale_size, log_bale_count);
            log->set_time_based_baling(log_time_based_baling, log_time_based_interval * 1000, manager->get_timer());
            log->setEnable(true);
         }
         router.bind(new router_type(this));
         if(!use_tcp_port)
         {
            serial_port.bind(new serial_port_type(router.get_handle(), serial_port_name, baud_rate));
            serial_port->set_log(log);
         }
         else
         {
            tcp_port.bind(new tcp_port_type(router.get_handle(), tcp_server_address, tcp_server_port));
            tcp_port->set_log(log);
         }
         router->set_log(log);
         if(serial_port != 0)
            router->register_port(serial_port.get_rep());
         if(tcp_port != 0)
            router->register_port(tcp_port.get_rep());
         if(pakbus_address == 0 || pakbus_address >= 4095)
         {
            connect_state = connect_state_searching;
            router->start_search();
         }
         else
         {
            if(serial_port != 0)
            {
               if(neighbour_address == 0 || neighbour_address >= 4095)
                  router->add_static_route(pakbus_address, serial_port.get_rep(), pakbus_address, 1);
               else
                  router->add_static_route(pakbus_address, serial_port.get_rep(), neighbour_address, 1);
            }
            if(tcp_port != 0)
            {
               if(neighbour_address == 0 || neighbour_address >= 4095)
                  router->add_static_route(pakbus_address, tcp_port.get_rep(), pakbus_address, 1);
               else
                  router->add_static_route(pakbus_address, tcp_port.get_rep(), neighbour_address, 1);
            }
            connect_state = connect_state_getting_table_defs;
            add_operation(new Bmp5SourceHelpers::OpGetTableDefs(this));
         }
         scheduler.bind(new Scheduler(manager->get_timer()));
      } // connect


      void Bmp5Source::disconnect()
      {
         router.clear();
         serial_port.clear();
         tcp_port.clear();
         connect_state = connect_state_offline;
         operations.clear();
      } // disconnect


      bool Bmp5Source::is_connected() const
      { return connect_state != connect_state_offline; }


      void Bmp5Source::get_properties(Csi::Xml::Element &prop_xml)
      {
         prop_xml.set_attr_str(serial_port_name, serial_port_name_name);
         prop_xml.set_attr_uint4(baud_rate, baud_rate_name);
         prop_xml.set_attr_bool(use_tcp_port, use_tcp_port_name);
         prop_xml.set_attr_str(tcp_server_address, tcp_server_address_name);
         prop_xml.set_attr_uint2(tcp_server_port, tcp_server_port_name);
         prop_xml.set_attr_uint2(my_pakbus_address, my_address_name);
         prop_xml.set_attr_uint2(pakbus_address, pakbus_address_name);
         prop_xml.set_attr_uint2(neighbour_address, neighbour_address_name);
         prop_xml.set_attr_str(pakbus_encryption_key, pakbus_encryption_key_name);
         prop_xml.set_attr_uint2(security_code, security_code_name);
         prop_xml.set_attr_uint4(poll_interval, poll_interval_name);
         prop_xml.set_attr_lgrdate(poll_base, poll_base_name);
         if(log_enabled)
         {
            Csi::Xml::Element::value_type log_xml(prop_xml.add_element(log_name));
            log_xml->set_attr_str(log_path, log_path_name);
            log_xml->set_attr_str(log_file, log_file_name);
            log_xml->set_attr_uint4(log_bale_size, log_bale_size_name);
            log_xml->set_attr_uint4(log_bale_count, log_bale_count_name);
            log_xml->set_attr_bool(log_time_based_baling, log_time_based_baling_name);
            log_xml->set_attr_int8(log_time_based_interval, log_time_based_interval_name);
         }
      } // get_properties


      void Bmp5Source::set_properties(Csi::Xml::Element &prop_xml)
      {
         Csi::Xml::Element::iterator li(prop_xml.find(log_name));
         serial_port_name = prop_xml.get_attr_str(serial_port_name_name);
         if(prop_xml.has_attribute(baud_rate_name))
            baud_rate = prop_xml.get_attr_uint4(baud_rate_name);
         else
            baud_rate = 115200;
         if(prop_xml.has_attribute(use_tcp_port_name))
            use_tcp_port = prop_xml.get_attr_bool(use_tcp_port_name);
         else
            use_tcp_port = false;
         if(prop_xml.has_attribute(tcp_server_address_name))
            tcp_server_address = prop_xml.get_attr_str(tcp_server_address_name);
         else
            tcp_server_address.cut(0);
         if(prop_xml.has_attribute(tcp_server_port_name))
            tcp_server_port = prop_xml.get_attr_uint2(tcp_server_port_name);
         else
            tcp_server_port = 6785;
         if(prop_xml.has_attribute(my_address_name))
            my_pakbus_address = prop_xml.get_attr_uint2(my_address_name);
         else
            my_pakbus_address = 4080;
         if(prop_xml.has_attribute(pakbus_address_name))
            pakbus_address = prop_xml.get_attr_uint2(pakbus_address_name);
         else
            pakbus_address = 0;
         if(prop_xml.has_attribute(neighbour_address_name))
            neighbour_address = prop_xml.get_attr_uint2(neighbour_address_name);
         else
            neighbour_address = 0;
         if(prop_xml.has_attribute(security_code_name))
            security_code = prop_xml.get_attr_uint2(security_code_name);
         else
            security_code = 0;
         if(prop_xml.has_attribute(pakbus_encryption_key_name))
            pakbus_encryption_key = prop_xml.get_attr_str(pakbus_encryption_key_name);
         else
            pakbus_encryption_key.cut(0);
         if(prop_xml.has_attribute(poll_interval_name))
            poll_interval = prop_xml.get_attr_uint4(poll_interval_name);
         else
            poll_interval = 5000;
         if(prop_xml.has_attribute(poll_base_name))
            poll_base = prop_xml.get_attr_lgrdate(poll_base_name);
         else
            poll_base = 0;
         if(li != prop_xml.end())
         {
            Csi::Xml::Element::value_type &log_xml(*li);
            log_enabled = true;
            log_path = log_xml->get_attr_str(log_path_name);
            log_file = log_xml->get_attr_str(log_file_name);
            if(log_xml->has_attribute(log_bale_size_name))
               log_bale_size = log_xml->get_attr_uint4(log_bale_size_name);
            else
               log_bale_size = 1200000;
            if(log_xml->has_attribute(log_bale_count_name))
               log_bale_count = log_xml->get_attr_uint4(log_bale_count_name);
            else
               log_bale_count = 5;
            if(log_xml->has_attribute(log_time_based_baling_name))
               log_time_based_baling = log_xml->get_attr_bool(log_time_based_baling_name);
            else
               log_time_based_baling = false;
            if(log_xml->has_attribute(log_time_based_interval_name))
               log_time_based_interval = log_xml->get_attr_int8(log_time_based_interval_name);
            else
               log_time_based_interval = 86400;
         }
         else
         {
            log_enabled = false;
            log_path.cut(0);
            log_file.cut(0);
            log_bale_size = 1200000;
            log_bale_count = 5;
            log_time_based_baling = false;
            log_time_based_interval = 86400;
         }
      } // set_properties


      void Bmp5Source::add_request(request_handle &request, bool more_to_follow)
      {
         // we need to find an operation that can accept this request.
         if(connect_state == connect_state_connected)
         {
            using namespace Bmp5SourceHelpers;
            Csi::PolySharedPtr<SourceOperation, OpDataRequest> request_op;
            request->set_wart(new TableFieldUri(request->get_uri()));
            for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
            {
               operation_handle &operation(*oi);
               if(operation->supports_data_requests())
               {
                  Csi::PolySharedPtr<SourceOperation, OpDataRequest> candidate(operation);
                  if(candidate->add_request(request))
                     request_op = candidate;
               }
            }
            if(request_op == 0)
            {
               request_op.bind(new OpDataRequest(this, request));
               add_operation(request_op.get_handle());
            }
            if(!more_to_follow)
               request_op->poll();
         }
         else
         {
            request->set_state(this, Request::state_error);
            set_retry_requests();
         }
      } // add_request


      void Bmp5Source::remove_request(request_handle &request)
      {
         using namespace Bmp5SourceHelpers;
         for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
         {
            operation_handle operation(*oi);
            if(operation->supports_data_requests())
            {
               Csi::PolySharedPtr<SourceOperation, OpDataRequest> request_op(operation);
               if(request_op->remove_request(request))
               {
                  if(request_op->get_requests_count() == 0)
                     remove_operation(operation.get_rep());
                  break;
               }
            }
         }
      } // remove_request


      void Bmp5Source::remove_all_requests()
      {
         operations_type temp(operations);
         for(operations_type::iterator oi = temp.begin(); oi != temp.end(); ++oi)
         {
            operation_handle operation(*oi);
            if(operation->supports_data_requests())
               remove_operation(operation.get_rep());
         }
      } // remove_all_requests


      void Bmp5Source::activate_requests()
      {
         // we will iterate all pending request operations and call their poll methods.
         using namespace Bmp5SourceHelpers;
         for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
         {
            operation_handle &operation(*oi);
            if(operation->supports_data_requests())
            {
               Csi::PolySharedPtr<SourceOperation, OpDataRequest> request_op(operation);
               request_op->poll();
            }
         }
      } // activate_requests


      void Bmp5Source::stop()
      {
      } // stop


      void Bmp5Source::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         timer = manager->get_timer();
      } // set_manager


      bool Bmp5Source::start_set_value(
         SinkBase *sink, StrUni const &uri, ValueSetter const &value)
      {
         add_operation(new Bmp5SourceHelpers::OpSetValue(this, sink, uri, value));
         return true;
      } // start_set_value


      bool Bmp5Source::start_send_file(
         SinkBase *sink, StrUni const &uri, StrUni const &dest_file_name, StrUni const &file_name)
      {
         add_operation(new Bmp5SourceHelpers::OpSendFile(this, sink, uri, dest_file_name, file_name));
         return true;
      } // start_send_file


      bool Bmp5Source::start_get_newest_file(
         SinkBase *sink, StrUni const &uri, StrUni const &pattern)
      {
         add_operation(new Bmp5SourceHelpers::OpNewestFile(this, sink, uri, pattern));
         return true;
      } // start_get_newest_file


      bool Bmp5Source::start_clock_check(
         SinkBase *sink, StrUni const &uri, bool should_set, bool send_server_time, Csi::LgrDate const &server_time)
      {
         add_operation(new Bmp5SourceHelpers::OpClock(this, sink, uri, should_set, send_server_time, server_time));
         return true;
      } // start_clock_check


      bool Bmp5Source::start_file_control(
         SinkBase *sink,
         StrUni const &uri,
         uint4 command,
         StrAsc const &p1,
         StrAsc const &p2)
      {
         add_operation(new Bmp5SourceHelpers::OpFileControl(this, sink, uri, command, p1, p2));
         return true;
      } // start_file_control


      bool Bmp5Source::start_terminal(
         TerminalSinkBase *sink, StrUni const &uri, int8 sink_token)
      {
         bool rtn(true);
         for(operations_type::iterator oi = operations.begin(); rtn && oi != operations.end(); ++oi)
         {
            operation_handle &operation(*oi);
            if(operation->supports_terminal())
               rtn = false;
         }
         if(rtn)
            add_operation(new Bmp5SourceHelpers::OpTerminal(this, sink, uri, sink_token));
         return rtn;
      } // start_terminal


      void Bmp5Source::send_terminal(
         TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len)
      {
         // we need to find the terminal operation.
         using namespace Bmp5SourceHelpers;
         for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
         {
            operation_handle &operation(*oi);
            if(operation->supports_terminal())
            {
               Csi::PolySharedPtr<SourceOperation, OpTerminal> terminal(operation);
               if(terminal->sink == sink && terminal->sink_token == sink_token)
                  terminal->send_data(buff, buff_len);
               break;
            }
         }
      } // send_terminal


      void Bmp5Source::close_terminal(
         TerminalSinkBase *sink, int8 sink_token)
      {
         // we need to search for the terminal operation.
         using namespace Bmp5SourceHelpers;
         for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
         {
            operation_handle &operation(*oi);
            if(operation->supports_terminal())
            {
               Csi::PolySharedPtr<SourceOperation, OpTerminal> terminal(operation);
               if(terminal->sink == sink && terminal->sink_token == sink_token)
                  remove_operation(operation.get_rep());
               break;
            }
         }
      } // close_terminal


      bool Bmp5Source::start_get_access_level(SinkBase *sink)
      {
         bool rtn(true);
         add_operation(new Bmp5SourceHelpers::OpCheckAccessLevel(this, sink));
         return rtn;
      } // start_get_access_level


      Bmp5Source::devconfig_session_handle Bmp5Source::make_devconfig_session(
         devconfig_library_handle library)
      {
         devconfig_session_handle rtn;
         try
         {
            if(router != 0 && pakbus_address != 0)
               rtn.bind(new Csi::DevConfig::PakbusSession(library, router.get_handle(), pakbus_address));
         }
         catch(std::exception &)
         {
            rtn.clear();
         }
         return rtn;
      } // make_devconfig_session


      bool Bmp5Source::start_list_files(
         SinkBase *sink, StrUni const &station_uri, int8 transaction, StrAsc const &filter)
      {
         add_operation(new Bmp5SourceHelpers::OpListFiles(this, sink, station_uri, transaction, filter));
         return true;
      } // start_list_files
      
      
      void Bmp5Source::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         TableFieldUri uri(uri_);
         symbols.clear();
         if(uri.get_source_name() == get_name())
         {
            symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_bmp5_source));
            if(uri.get_table_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
               if(uri.get_column_name().length() > 0)
                  symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
            }
         }
      } // breakdown_uri


      Bmp5Source::symbol_handle Bmp5Source::get_source_symbol()
      {
         return source_symbol;
      } // get_source_symbol


      void Bmp5Source::add_operation(operation_handle operation)
      {
         operations.push_back(operation);
         operation->start();
      } // add_operation
      

      void Bmp5Source::remove_operation(operation_type *operation_, uint4 reboot_interval)
      {
         operations_type::iterator oi(
            std::find_if(operations.begin(), operations.end(), Csi::HasSharedPtr<operation_type>(operation_)));
         if(oi != operations.end())
         {
            operation_handle operation(*oi);
            operations.erase(oi);
            operation->close_transaction();
         }
         if(reboot_interval && disable_comms_id == 0)
         {
            if(serial_port != 0)
               serial_port->on_comm_enabled_change(false);
            if(tcp_port != 0)
               tcp_port->on_comm_enabled_change(false);
            disable_comms_id = timer->arm(this, reboot_interval);
         }
      } // remove_operation


      void Bmp5Source::set_pakbus_address(uint2 value)
      {
         if(connect_state == connect_state_searching && value != pakbus_address)
         {
            pakbus_address = value;
            neighbour_address = value;
            connect_state = connect_state_getting_table_defs;
            add_operation(new Bmp5SourceHelpers::OpGetTableDefs(this));
         }
      } // set_pakbus_address


      void Bmp5Source::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            if(serial_port != 0)
               serial_port->on_comm_enabled_change(true);
            if(tcp_port != 0)
               tcp_port->on_comm_enabled_change(true);
            connect();
         }
         else if(id == disable_comms_id)
         {
            disable_comms_id = 0;
            if(serial_port != 0)
               serial_port->on_comm_enabled_change(true);
            if(tcp_port != 0)
               tcp_port->on_comm_enabled_change(true);
            connect_state = connect_state_getting_table_defs;
            add_operation(new Bmp5SourceHelpers::OpGetTableDefs(this));
         }
         else if(id == retry_requests_id)
         {
            retry_requests_id = 0;
            for(Manager::requests_iterator ri = manager->requests_begin();
                ri != manager->requests_end(); ++ri)
            {
               request_handle &request(*ri);
               if(request->get_source() == static_cast<SourceBase *>(this) &&
                  request->get_state() == Request::state_error)
                  add_request(request, true);
            }
         }
      } // onOneShotFired


      void Bmp5Source::onScheduledEvent(uint4 id)
      {
         if(id == poll_schedule_id)
         {
            for(operations_type::iterator oi = operations.begin(); oi != operations.end(); ++oi)
            {
               operation_handle &operation(*oi);
               if(operation->supports_data_requests())
                  static_cast<Bmp5SourceHelpers::OpDataRequest *>(operation.get_rep())->poll();
            }
         }
      } // onScheduledEvent

      
      void Bmp5Source::on_connect_failure(StrAsc const &reason)
      {
         manager->report_source_log(this, reason);
         connect_state = connect_state_offline;
         router->shut_down();
         operations.clear();
         retry_id = timer->arm(this, 10000);
         if(poll_schedule_id)
            scheduler->cancel(poll_schedule_id);
         poll_schedule_id = 0;
      } // on_connect_failure


      void Bmp5Source::on_connect_complete()
      {
         connect_state = connect_state_connected;
         manager->report_source_connect(this);
         if(retry_requests_id)
            timer->disarm(retry_requests_id);
         if(is_started())
            start();
         poll_schedule_id = scheduler->start(this, poll_base, poll_interval, false);
      } // on_connect_complete


      void Bmp5Source::set_retry_requests()
      {
         if(retry_requests_id == 0)
            retry_requests_id = timer->arm(this, 10000);
      } // set_retry_requests
   };
};

