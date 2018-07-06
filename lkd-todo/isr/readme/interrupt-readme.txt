There are several interesting functions in preempt_mask.h:

    in_irq(): hardware interrupt
    in_softirq(): Are we in a softirq context?
    in_interrupt(): Interrupt context?
    in_nmi(): Are we in NMI context?
    in_atomic(): Are we running in atomic context? WARNING: this macro cannot always detect atomic context

