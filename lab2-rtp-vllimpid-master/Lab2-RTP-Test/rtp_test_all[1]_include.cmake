if(EXISTS "/home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/rtp_test_all[1]_tests.cmake")
  include("/home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/rtp_test_all[1]_tests.cmake")
else()
  add_test(rtp_test_all_NOT_BUILT rtp_test_all_NOT_BUILT)
endif()
