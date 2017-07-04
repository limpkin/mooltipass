package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.PcbWay;

public class PcbWaySteps {
	PcbWay pcb= new PcbWay(WebDriverFactory.get());

	@When("I login PcbWay with '(.*)'")
	public void login(String username){
		pcb.goToLogin();
		pcb.enterEmail(username);
		String password =System.getenv().get("PCBPASS");
		pcb.enterPassword(password);
		pcb.submit();
		
	}
	@When("I go to PcbWay login page")
	public void pressLogin(){
		pcb.goToLogin();
		Assert.assertTrue("Expected to be at login page", pcb.checkAtLoginPage());
		
	}
	@Then("I should be logged in PcbWay")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",pcb.checkLogin());
	}
	
	@When("I logout PcbWay")
	public void pressLogout(){
		pcb.goTodDashboard();
		pcb.logout();
	}
}
