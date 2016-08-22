TEMPLATE=subdirs
SUBDIRS=\
   qsslcertificate \
   qsslcipher \
   qsslellipticcurve \
   qsslerror \
   qsslkey \

qtConfig(ssl)|qtConfig(openssl)|qtConfig(openssl-linked) {
    qtConfig(private_tests) {
        SUBDIRS += \
            qsslsocket \
            qsslsocket_onDemandCertificates_member \
            qsslsocket_onDemandCertificates_static \
    }
}

winrt: SUBDIRS -= \
   qsslsocket_onDemandCertificates_member \
   qsslsocket_onDemandCertificates_static \

qtConfig(ssl)|qtConfig(openssl)|qtConfig(openssl-linked) {
    qtConfig(private_tests) {
        SUBDIRS += qasn1element \
                   qssldiffiehellmanparameters
    }
}
