cmake_minimum_required(VERSION 3.16)
project(trader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)
#add_definitions(-DSIMULATED_TRADING ) #-DOFFLINE_MODE

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

set(HEADERS
        model/IB/ats_ib_account.h
        model/IB/ats_ib_client.hpp
        model/IB/ats_ib_data_provider.h
        model/IB/ats_ib_downloader.h
        model/IB/ats_ib_events.h
        model/IB/ats_ib_orders.h
        model/ats_application.h
        model/ats_av_data_provider.h
        model/ats_configuration.h
        model/ats_db_client.h
        model/interface/ats_idata_provider.h
        model/ats_optimizer.h
        model/ats_order_request.h
        model/ats_utils.h
        model/processing/ats_dsp_algorithms.h
        model/interface/ats_iprocessor.h
        model/processing/ats_multiprocessor.h
        model/processing/ats_processing_cluster.h
        model/processing/ats_processor.h
        model/processing/strategies/ats_aroon_atr_ma_strategy.h
        model/processing/strategies/ats_aroon_strategy.h
        model/interface/ats_istrategy.h
        model/processing/strategies/ats_ma_strategy.h
        model/processing/strategies/ats_macd_adx_strategy.h
        model/processing/strategies/ats_macd_strategy.h
        thirdparty/Avapi/AvapiConnection.hpp
        thirdparty/Avapi/lib_avapi.hpp
        thirdparty/IB/client/CommissionReport.h
        thirdparty/IB/client/CommonDefs.h
        thirdparty/IB/client/Contract.h
        thirdparty/IB/client/ContractCondition.h
        thirdparty/IB/client/DefaultEWrapper.h
        thirdparty/IB/client/DepthMktDataDescription.h
        thirdparty/IB/client/EClient.h
        thirdparty/IB/client/EClientMsgSink.h
        thirdparty/IB/client/EClientSocket.h
        thirdparty/IB/client/EDecoder.h
        thirdparty/IB/client/EMessage.h
        thirdparty/IB/client/EMutex.h
        thirdparty/IB/client/EOrderDecoder.h
        thirdparty/IB/client/EPosixClientSocketPlatform.h
        thirdparty/IB/client/EReader.h
        thirdparty/IB/client/EReaderOSSignal.h
        thirdparty/IB/client/EReaderSignal.h
        thirdparty/IB/client/ESocket.h
        thirdparty/IB/client/ETransport.h
        thirdparty/IB/client/EWrapper.h
        thirdparty/IB/client/EWrapper_prototypes.h
        thirdparty/IB/client/Execution.h
        thirdparty/IB/client/FamilyCode.h
        thirdparty/IB/client/HistogramEntry.h
        thirdparty/IB/client/HistoricalTick.h
        thirdparty/IB/client/HistoricalTickBidAsk.h
        thirdparty/IB/client/HistoricalTickLast.h
        thirdparty/IB/client/IExternalizable.h
        thirdparty/IB/client/MarginCondition.h
        thirdparty/IB/client/NewsProvider.h
        thirdparty/IB/client/OperatorCondition.h
        thirdparty/IB/client/Order.h
        thirdparty/IB/client/OrderCondition.h
        thirdparty/IB/client/OrderState.h
        thirdparty/IB/client/PercentChangeCondition.h
        thirdparty/IB/client/PriceCondition.h
        thirdparty/IB/client/PriceIncrement.h
        thirdparty/IB/client/ScannerSubscription.h
        thirdparty/IB/client/SoftDollarTier.h
        thirdparty/IB/client/StdAfx.h
        thirdparty/IB/client/TagValue.h
        thirdparty/IB/client/TickAttrib.h
        thirdparty/IB/client/TickAttribBidAsk.h
        thirdparty/IB/client/TickAttribLast.h
        thirdparty/IB/client/TimeCondition.h
        thirdparty/IB/client/TwsSocketClientErrors.h
        thirdparty/IB/client/VolumeCondition.h
        thirdparty/IB/client/bar.h
        thirdparty/IB/client/executioncondition.h
        thirdparty/IB/client/platformspecific.h
        thirdparty/IB/client/resource.h
        thirdparty/gaussianblur.h
        thirdparty/nlohmann/nlohmann_fwd.hpp
        thirdparty/qcustomplot.h
        ui/ats_axis_ticker_date_time.h
        ui/ats_dlg_downloader.h
        ui/ats_dlg_magnifier.h
        ui/ats_dlg_optimizer.h
        ui/ats_dlg_portfolio.h
        ui/ats_dlg_settings.h
        ui/ats_ui_utils.h
        ui/ats_wdg_plot_indicator.h
        ui/ats_wdg_plot_main.h
        ui/ats_wdg_plots.h
        ui/ats_wnd_main.h
        model/processing/strategies/ats_mama_strategy.hpp
        model/interface/ats_ijson.hpp
        )

set(SOURCES
        main.cpp
        model/IB/ats_ib_account.cpp
        model/IB/ats_ib_client.cpp
        model/IB/ats_ib_data_provider.cpp
        model/IB/ats_ib_downloader.cpp
        model/IB/ats_ib_events.cpp
        model/IB/ats_ib_orders.cpp
        model/ats_application.cpp
        model/ats_av_data_provider.cpp
        model/ats_configuration.cpp
        model/ats_db_client.cpp
        model/ats_optimizer.cpp
        model/ats_order_request.cpp
        model/ats_utils.cpp
        model/processing/ats_dsp_algorithms.cpp
        model/processing/ats_multiprocessor.cpp
        model/processing/ats_processing_cluster.cpp
        model/processing/ats_processor.cpp
        model/processing/strategies/ats_aroon_atr_ma_strategy.cpp
        model/processing/strategies/ats_aroon_strategy.cpp
        model/processing/strategies/ats_ma_strategy.cpp
        model/processing/strategies/ats_macd_adx_strategy.cpp
        model/processing/strategies/ats_macd_strategy.cpp
        thirdparty/IB/client/ContractCondition.cpp
        thirdparty/IB/client/DefaultEWrapper.cpp
        thirdparty/IB/client/EClient.cpp
        thirdparty/IB/client/EClientSocket.cpp
        thirdparty/IB/client/EDecoder.cpp
        thirdparty/IB/client/EMessage.cpp
        thirdparty/IB/client/EMutex.cpp
        thirdparty/IB/client/EOrderDecoder.cpp
        thirdparty/IB/client/EReader.cpp
        thirdparty/IB/client/EReaderOSSignal.cpp
        thirdparty/IB/client/ESocket.cpp
        thirdparty/IB/client/MarginCondition.cpp
        thirdparty/IB/client/OperatorCondition.cpp
        thirdparty/IB/client/OrderCondition.cpp
        thirdparty/IB/client/PercentChangeCondition.cpp
        thirdparty/IB/client/PriceCondition.cpp
        thirdparty/IB/client/SoftDollarTier.cpp
        thirdparty/IB/client/StdAfx.cpp
        thirdparty/IB/client/TimeCondition.cpp
        thirdparty/IB/client/VolumeCondition.cpp
        thirdparty/IB/client/executioncondition.cpp
        thirdparty/qcustomplot.cpp
        ui/ats_axis_ticker_date_time.cpp
        ui/ats_dlg_downloader.cpp
        ui/ats_dlg_magnifier.cpp
        ui/ats_dlg_optimizer.cpp
        ui/ats_dlg_portfolio.cpp
        ui/ats_dlg_settings.cpp
        ui/ats_ui_utils.cpp
        ui/ats_wdg_plot_indicator.cpp
        ui/ats_wdg_plot_main.cpp
        ui/ats_wdg_plots.cpp
        ui/ats_wnd_main.cpp
        model/processing/strategies/ats_mama_strategy.cpp)


add_executable(trader ${HEADERS} ${SOURCES})

target_link_libraries(trader Qt5::Core)
target_link_libraries(trader Qt5::Widgets)
target_link_libraries(trader Qt5::Network)
target_link_libraries(trader Qt5::PrintSupport)
target_link_libraries(trader ta_lib)
target_link_libraries(trader mongoclient)