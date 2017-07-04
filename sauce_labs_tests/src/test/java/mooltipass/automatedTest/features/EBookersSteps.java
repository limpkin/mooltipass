package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.DropBox;
import mooltipass.automatedTest.pageObjects.EBookers;

public class EBookersSteps {
	EBookers ebookers= new EBookers(WebDriverFactory.get());

	@When("I login EBookers with '(.*)'")
	public void login(String username){
		ebookers.goTodDashboard();
		ebookers.goToLogin();
		ebookers.enterEmail(username);
		String password=System.getenv().get("EBOOKPASS");
		ebookers.enterPassword(password);
		ebookers.submit();
		
	}
	@When("I go to EBookers login page")
	public void pressLogin(){
		ebookers.goTodDashboard();
		ebookers.goToLogin();
//		Assert.assertTrue("Expected to be at login page", ebookers.checkAtLoginPage());
		
	}
	@Then("I should be logged in EBookers")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",ebookers.checkLogin());
	}
	
	@When("I logout EBookers")
	public void pressLogout(){
		ebookers.goTodDashboard();
		ebookers.logout();
	}
}
